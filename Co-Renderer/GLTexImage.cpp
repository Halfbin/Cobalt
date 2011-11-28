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
  void GLTexImage::load_map (uint level, const void* data, TexFormat format, uint width, uint height, uptr size)
  {
    Rk::require (data != 0, "data is null");
    Rk::require (format < texformat_count, "format not valid");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

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

    if (!gl_formats [format][2]) // compressed
    {
      glCompressedTexImage2D (
        target,
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
        target,
        level,
        gl_formats [format][0],
        width, height, 0,
        gl_formats [format][1],
        gl_formats [format][2],
        data
      );
      check_gl ("glTexImage2D");
    }
  }
  // load_map

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
  GLTexImage::GLTexImage (uint level_count, TexImageWrap wrap, TexImageType type)
  {
    if (level_count == 0)
      throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - level_count is zero");

    u32 min_filter,
        mag_filter;

    switch (type)
    {
      case teximage_rect:
        if (level_count != 1)
          throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - level_count must be 1 for rectangular textures");
        target = GL_TEXTURE_RECTANGLE;
        min_filter = GL_NEAREST;
        mag_filter = GL_NEAREST;
      break;

      case teximage_normal:
        target = GL_TEXTURE_2D;
        min_filter = GL_LINEAR_MIPMAP_LINEAR;
        mag_filter = GL_LINEAR;
      break;

      default:
        throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - invalid type");
    }

    u32 gl_wrap = 0;
    switch (wrap)
    {
      case teximage_wrap:
        gl_wrap = GL_REPEAT;
      break;

      case teximage_clamp:
        gl_wrap = GL_CLAMP_TO_EDGE;
      break;

      default:
        throw Rk::Exception ("Co-GLRenderer: IxRenderContext::create_tex_image - invalid wrapping mode");
    }

    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    glTexParameteri (target, GL_TEXTURE_MAX_LEVEL, level_count - 1);
    glTexParameteri (target, GL_TEXTURE_WRAP_S, gl_wrap);
    glTexParameteri (target, GL_TEXTURE_WRAP_T, gl_wrap);
    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, mag_filter);

    glBindTexture (target, 0);
  }

} // namespace Co
