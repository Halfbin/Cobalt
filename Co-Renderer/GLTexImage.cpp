//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLTexImage.hpp"

// Uses
#include "GL.hpp"

namespace Co
{
  struct GLFormat
  {
    GLenum iformat,
           format,
           type;

    bool compressed () const { return !type; }

  };

  static const GLFormat gl_formats [texformat_count] = {
    { GL_RGB,        GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 }, // rgb565
    { GL_RGB,        GL_BGR,  GL_UNSIGNED_SHORT_5_6_5 }, // bgr565
    { GL_SRGB,       GL_RGB,  GL_UNSIGNED_BYTE        }, // rgb888
    { GL_SRGB,       GL_BGR,  GL_UNSIGNED_BYTE        }, // bgr888
    { GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE        }, // rgba8888
    { GL_SRGB_ALPHA, GL_BGRA, GL_UNSIGNED_BYTE        }, // bgra8888

    { GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,       0, 0 }, // bc1/dxt1,
    { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0, 0 }, // bc1/dxt1a
    { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0, 0 }, // bc2/dxt3,
    { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0, 0 }, // bc3/dxt5,
    { GL_COMPRESSED_RG_RGTC2,                 0, 0 }, // bc5/dxtn/3dc/ati2,

    { GL_R8, GL_RED, GL_UNSIGNED_BYTE }, // r8
    { GL_R8, GL_RED, GL_UNSIGNED_BYTE }  // a8 (swizzled r8)
  };

  //
  // = Mipmap loading ==================================================================================================
  //
  uptr GLTexImage::load_map (u32 sub_image, u32 level, const void* data, uptr available)
  {
    if (!data)
      throw std::invalid_argument ("data is null");

    if (sub_image >= layer_count)
      throw std::out_of_range ("Texture sub-image index out of range");

    u32 level_width  = width  >> level,
        level_height = height >> level,
        level_area   = level_width * level_height;

    uptr size = level_area;
    
    switch (format)
    {
      case texformat_rgb565:
      case texformat_bgr565:
        size *= 2;
      break;

      case texformat_rgb888:
      case texformat_bgr888:
        size *= 3;
      break;

      case texformat_rgba8888:
      case texformat_bgra8888:
        size *= 4;
      break;

      case texformat_dxt1:
      case texformat_dxt1a:
        size /= 2;
      break;

      default:; // Other formats are 1 byte per pixel anyway
    }

    if (available < size)
      throw std::length_error ("Not enough data available to load texture map");

    GLenum sub_target = target;

    if (target == GL_TEXTURE_CUBE_MAP)
    {
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

      sub_target = faces [sub_image];
    }

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    const auto& gl_fmt = gl_formats [format];

    if (target == GL_TEXTURE_2D_ARRAY)
    {
      if (gl_fmt.compressed ()) // compressed
      {
        glCompressedTexSubImage3D (
          target,
          level,
          0, 0, sub_image,
          level_width, level_height, 1,
          gl_fmt.iformat,
          (GLsizei) size,
          data
        );
        check_gl ("glCompressedTexSubImage3D");
      }
      else // uncompressed
      {
        glTexSubImage3D (
          target,
          level,
          0, 0, sub_image,
          level_width, level_height, 1,
          gl_fmt.format,
          gl_fmt.type,
          data
        );
        check_gl ("glTexSubImage3D");
      }
    }
    else // non-array
    {
      if (gl_fmt.compressed ()) // compressed
      {
        glCompressedTexImage2D (
          sub_target,
          level,
          gl_fmt.iformat,
          level_width, level_height, 0,
          (GLsizei) size,
          data
        );
        check_gl ("glCompressedTexImage2D");
      }
      else // uncompressed
      {
        glTexImage2D (
          sub_target,
          level,
          gl_fmt.iformat,
          level_width, level_height, 0,
          gl_fmt.format,
          gl_fmt.type,
          data
        );
        check_gl ("glTexImage2D");

        if (format == texformat_a8)
        {
          int swizzle [4] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
          glTexParameteriv (sub_target, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
          check_gl ("glTexParameteriv");
        }
      }
    } // !array

    return size;
  } // load_map

  //
  // Destructor
  //
  GLTexImage::~GLTexImage ()
  {
    glDeleteTextures (1, &name);
  }

  //
  // = 2D Constructor ==================================================================================================
  //
  GLTexImage::GLTexImage (
    TexFormat    new_format,
    u32          new_width,
    u32          new_height,
    u32          level_count,
    TexImageWrap wrap,
    bool         min_filter,
    bool         mag_filter,
    const void*  data,
    uptr         size
  ) :
    format      (new_format),
    width       (new_width),
    height      (new_height),
    layer_count (1)
  {
    if (format >= texformat_count)
      throw std::invalid_argument ("Invalid texture format");

    if (width == 0 || height == 0)
      throw std::invalid_argument ("Invalid texture dimensions");

    if (level_count == 0)
      throw std::invalid_argument ("Cannot create texture with zero levels");

    if (wrap >= 2)
      throw std::invalid_argument ("Invalid texture wrapping mode");

    if (size && !data)
      throw std::invalid_argument ("Nonzero data size with null data");

    target = GL_TEXTURE_2D;

    GLenum gl_min_filter = min_filter ? GL_LINEAR : GL_NEAREST,
           gl_mag_filter = mag_filter ? GL_LINEAR : GL_NEAREST;

    if (level_count > 1 && min_filter)
      gl_min_filter = GL_LINEAR_MIPMAP_LINEAR;

    GLenum gl_wrap = (wrap == texwrap_clamp) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    
    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    glTexParameteri (target, GL_TEXTURE_MAX_LEVEL,  level_count - 1);
    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
    glTexParameteri (target, GL_TEXTURE_WRAP_S,     gl_wrap);
    glTexParameteri (target, GL_TEXTURE_WRAP_T,     gl_wrap);

    if (data)
    {
      auto byte_data = (const u8*) data;
      for (uint level = 0; level != level_count; level++)
        byte_data += load_map (0, level, byte_data, size);
    }

    glBindTexture (target, 0);
  }

  //
  // = Rectangle Constructor ===========================================================================================
  //
  GLTexImage::GLTexImage (
    TexFormat    new_format,
    u32          new_width,
    u32          new_height,
    TexImageWrap wrap,
    const void*  data,
    uptr         size
  ) :
    format      (new_format),
    width       (new_width),
    height      (new_height),
    layer_count (1)
  {
    if (format >= texformat_count)
      throw std::invalid_argument ("Invalid texture format");

    if (width == 0 || height == 0)
      throw std::invalid_argument ("Invalid texture dimensions");

    if (wrap >= 2)
      throw std::invalid_argument ("Invalid texture wrapping mode");

    if (size && !data)
      throw std::invalid_argument ("Nonzero data size with null data");

    target = GL_TEXTURE_RECTANGLE;

    GLenum gl_wrap = (wrap == texwrap_clamp) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    
    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    glTexParameteri (target, GL_TEXTURE_WRAP_S, gl_wrap);
    glTexParameteri (target, GL_TEXTURE_WRAP_T, gl_wrap);

    if (data)
      load_map (0, 0, data, size);

    glBindTexture (target, 0);
  }

  //
  // = Cubemap Constructor =============================================================================================
  //
  GLTexImage::GLTexImage (
    TexFormat  new_format,
    u32        new_width,
    u32        new_height,
    bool       min_filter,
    bool       mag_filter
  ) :
    format      (new_format),
    width       (new_width),
    height      (new_height),
    layer_count (6)
  {
    if (format >= texformat_count)
      throw std::invalid_argument ("Invalid texture format");

    if (width == 0 || height == 0)
      throw std::invalid_argument ("Invalid texture dimensions");

    target = GL_TEXTURE_CUBE_MAP;

    GLenum gl_min_filter = min_filter ? GL_LINEAR : GL_NEAREST,
           gl_mag_filter = mag_filter ? GL_LINEAR : GL_NEAREST;

    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
  }

  //
  // = Array Constructor ===============================================================================================
  //
  GLTexImage::GLTexImage (
    TexFormat    new_format,
    u32          new_width,
    u32          new_height,
    u32          new_layer_count,
    u32          level_count,
    TexImageWrap wrap,
    bool         min_filter,
    bool         mag_filter
  ) :
    format      (new_format),
    width       (new_width),
    height      (new_height),
    layer_count (new_layer_count)
  {
    if (format >= texformat_count)
      throw std::invalid_argument ("Invalid texture format");

    if (width == 0 || height == 0)
      throw std::invalid_argument ("Invalid texture dimensions");

    if (layer_count == 0)
      throw std::invalid_argument ("Cannot create texture array with zero layers");

    if (level_count == 0)
      throw std::invalid_argument ("Cannot create texture with zero levels");

    if (wrap >= 2)
      throw std::invalid_argument ("Invalid texture wrapping mode");

    target = GL_TEXTURE_2D_ARRAY;

    GLenum gl_min_filter = min_filter ? GL_LINEAR : GL_NEAREST,
           gl_mag_filter = mag_filter ? GL_LINEAR : GL_NEAREST;

    if (level_count > 1 && min_filter)
      gl_min_filter = GL_LINEAR_MIPMAP_LINEAR;

    GLenum gl_wrap = (wrap == texwrap_clamp) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    
    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (target, name);
    check_gl ("glBindTexture");

    glTexParameteri (target, GL_TEXTURE_MAX_LEVEL,  level_count - 1);
    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
    glTexParameteri (target, GL_TEXTURE_WRAP_S,     gl_wrap);
    glTexParameteri (target, GL_TEXTURE_WRAP_T,     gl_wrap);

    const auto& gl_fmt = gl_formats [format];

    if (gl_fmt.compressed ())
    {
      // I'm not sure we need to do anything here
      // CompressedTexImageND won't take a null pointer
    }
    else
    {
      for (uint level = 0; level != level_count; level++)
      {
        glTexImage3D (target, level, gl_fmt.iformat, width, height, layer_count, 0, gl_fmt.format, gl_fmt.type, nullptr);
        check_gl ("glTexImage3D");
      }
    }

    glBindTexture (target, 0);
  }

  /*GLTexImage::GLTexImage (uint level_count, uint layer_count, TexImageWrap wrap, bool min_filter, bool mag_filter, TexImageType type)
  {
    if (level_count == 0)
      throw std::invalid_argument ("level_count is zero");

    GLenum gl_min_filter = min_filter ? GL_LINEAR : GL_NEAREST,
           gl_mag_filter = mag_filter ? GL_LINEAR : GL_NEAREST;

    if (type == textype_rectangle)
    {
      if (level_count != 1)
        throw std::logic_error ("level_count must be 1 for rectangular textures");

      target = GL_TEXTURE_RECTANGLE;
    }
    else if (type == textype_2d)
    {
      target = GL_TEXTURE_2D;

      if (min_filter && level_count > 1)
        gl_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    }
    else if (type == textype_cube)
    {
      target = GL_TEXTURE_CUBE_MAP;
    }
    else if (type == textype_array)
    {
      target = GL_TEXTURE_2D_ARRAY;

      if (layer_count == 0)
        throw std::logic_error ("layer_count must be nonzero for array textures");

      if (min_filter && level_count > 1)
        gl_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    }
    else
    {
      throw std::invalid_argument ("Invalid type");
    }

    if (type != textype_array && layer_count != 0)
      throw std::logic_error ("layer_count must be 0 for non-array textures");

    GLenum gl_wrap;

    if (type == textype_cube)
    {
      if (min_filter || mag_filter)
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
          throw std::invalid_argument ("Invalid wrapping mode");
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

    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, gl_mag_filter);

    // God damnit
    if (type == textype_array)
    {
      for (uint level = 0; level != level_count; level++)
      {
        glTexImage3D (target, level, 
      }
    }

    glBindTexture (target, 0);
  }*/

} // namespace Co
