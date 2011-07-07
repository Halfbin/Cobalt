//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLTexture.hpp"

// Uses
#include "Common.hpp"

namespace Co
{
  //
  // Mipmap loading
  //
  void GLTexture::load_map (uint level, const void* data, TexFormat format, uint width, uint height, uptr size) try
  {
    if (!data)
      throw Rk::Violation ("data is null");
    
    if (format >= texformat_count)
      throw Rk::Violation ("format not valid");

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
    };

    if (!gl_formats [format][2]) // compressed
    {
      glCompressedTexImage2D (
        GL_TEXTURE_2D,
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
        GL_TEXTURE_2D,
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
  catch (...)
  {
    Rk::log_frame ("Co::GLTexture::load_map")
      << " level: "  << level
      << " format: " << format;
    throw;
  }
  // load_map

  //
  // Destructors
  GLTexture::~GLTexture ()
  {
    glDeleteTextures (1, &name);
  }

  void GLTexture::destroy ()
  {
    delete this;
  }

  //
  // Constructor
  //
  GLTexture::GLTexture (uint level_count, bool wrap) try
  {
    if (level_count == 0)
      throw Rk::Violation ("level_count is zero");

    glGenTextures (1, &name);
    check_gl ("glGenTextures");

    glBindTexture (GL_TEXTURE_2D, name);
    check_gl ("glBindTexture");

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level_count - 1);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture (GL_TEXTURE_2D, 0);
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLTexture::GLTexture")
      << " level_count: " << level_count;
  }

} // namespace Co
