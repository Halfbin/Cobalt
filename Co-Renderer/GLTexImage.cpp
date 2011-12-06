//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLTexImage.hpp"

// Uses
#include "GL.hpp"

namespace Co
{
  //
  // Mipmap loading
  //
  bool GLTexImage::load_map (u32 sub_image, u32 level, const void* data, TexFormat format, u32 width, u32 height, uptr size) try
  {
    if (!data)
      throw Rk::Exception ("Co-GLRenderer: IxTexImage::load_map - data is null");

    if (format >= texformat_count)
      throw Rk::Exception ("Co-GLRenderer: IxTexImage::load_map - invalid format");

    if (width == 0 || height == 0)
      throw Rk::Exception ("Co-GLRenderer: IxTexImage::load_map - invalid dimensions");

    GLenum gl_formats [texformat_count][3] = {
      { GL_RGB,  GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 }, // rgb565
      { GL_RGB,  GL_BGR,  GL_UNSIGNED_SHORT_5_6_5 }, // bgr565
      { GL_RGB,  GL_RGB,  GL_UNSIGNED_BYTE        }, // rgb888
      { GL_RGB,  GL_BGR,  GL_UNSIGNED_BYTE        }, // bgr888
      { GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8 }, // rgba8888
      { GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8 }, // bgra8888

      { GL_COMPRESSED_RGB_S3TC_DXT1_EXT,  0, 0 }, // bc1/dxt1,
      { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0 }, // bc1/dxt1a
      { GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0 }, // bc2/dxt3,
      { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0 }, // bc3/dxt5,
      { GL_COMPRESSED_RG_RGTC2,           0, 0 }, // bc5/dxtn/3dc/ati2,

      { GL_R8, GL_RED, GL_UNSIGNED_BYTE } // r8
    };

    GLenum image_target = target;

    if (target == GL_TEXTURE_CUBE_MAP)
    {
      if (sub_image >= 6)
        throw Rk::Exception ("Co-GLRenderer: IxTexImage::load_map - level >= 6 is invalid for cube maps");

      // Remember that OpenGL cube maps are left-handed z-forward
      // So much for coordinate independence
      static const GLenum faces [6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // front
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, // back
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // left
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, // right
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // top
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // bottom
      };

      image_target = faces [sub_image];
    }

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    if (!gl_formats [format][2]) // compressed
    {
      glCompressedTexImage2D (
        image_target,
        level,
        gl_formats [format][0],
        width, height, 0,
        size,
        data
      );
      check_gl ("glCompressedTexImage2D");
    }
    else
    {
      glTexImage2D (
        image_target,
        level,
        gl_formats [format][0],
        width, height, 0,
        gl_formats [format][1],
        gl_formats [format][2],
        data
      );
      check_gl ("glTexImage2D");
    }

    return true;
  }
  catch (...)
  {
    return false;
  } // load_map

  //
  // Destructors
  GLTexImage::~GLTexImage ()
  {
    glDeleteTextures (1, &name);
  }

  void GLTexImage::destroy ()
  {
    delete this;
  }

  //
  // Constructor
  //
  GLTexImage::GLTexImage (uint level_count, TexImageWrap wrap, bool filter, TexImageType type)
  {
    if (level_count == 0)
      throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - level_count is zero");

    GLenum min_filter,
           mag_filter;

    if (filter)
    {
      min_filter = GL_LINEAR;
      mag_filter = GL_LINEAR;
    }
    else
    {
      min_filter = GL_NEAREST;
      mag_filter = GL_NEAREST;
    }

    if (type == textype_rectangle)
    {
      if (level_count != 1)
        throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - level_count must be 1 for rectangular textures");

      target = GL_TEXTURE_RECTANGLE;
    }
    else if (type == textype_2d)
    {
      target = GL_TEXTURE_2D;

      if (filter && level_count > 1)
        min_filter = GL_LINEAR_MIPMAP_LINEAR;
    }
    else if (type == textype_cube)
    {
      target = GL_TEXTURE_CUBE_MAP;
    }
    else
    {
      throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - invalid type");
    }

    GLenum gl_wrap;

    if (type == textype_cube)
    {
      if (filter)
        gl_wrap = GL_CLAMP_TO_BORDER;
      else
        gl_wrap = GL_CLAMP_TO_EDGE;
    }
    else
    {
      switch (wrap)
      {
        case texwrap_wrap:
          gl_wrap = GL_REPEAT;
        break;

        case texwrap_clamp:
          gl_wrap = GL_CLAMP_TO_EDGE;
        break;

        default:
          throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - invalid wrapping mode");
      }
    }

    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    glTexParameteri (target, GL_TEXTURE_MAX_LEVEL, level_count - 1);

    glTexParameteri (target, GL_TEXTURE_WRAP_S, gl_wrap);
    glTexParameteri (target, GL_TEXTURE_WRAP_T, gl_wrap);
    if (type == textype_cube)
      glTexParameteri (target, GL_TEXTURE_WRAP_S, gl_wrap);

    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, mag_filter);

    glBindTexture (target, 0);
  }

} // namespace Co
