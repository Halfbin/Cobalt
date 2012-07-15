//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLTEXIMAGE
#define CO_GLRENDERER_H_GLTEXIMAGE

// Implements
#include <Co/TexImage.hpp>

// Uses
#include <Co/RenderContext.hpp>

#include "GL.hpp"

namespace Co
{
  class GLTexImage :
    public TexImage
  {
    u32       name,
              target,
              width,
              height,
              layer_count;
    TexFormat format;
    
  protected:
    virtual uptr load_map (u32 sub_image, u32 level, const void* data, uptr size = 0);

  public:
    typedef std::shared_ptr <GLTexImage> Ptr;

    GLTexImage (
      TexFormat    format,
      u32          width,
      u32          height,
      u32          level_count,
      TexImageWrap wrap,
      bool         min_filter,
      bool         mag_filter,
      const void*  data = nullptr,
      uptr         size = 0
    );

    GLTexImage (
      TexFormat    format,
      u32          width,
      u32          height,
      TexImageWrap wrap,
      const void*  data = nullptr,
      uptr         size = 0
    );

    GLTexImage (
      TexFormat format,
      u32       width,
      u32       height,
      bool      min_filter,
      bool      mag_filter
    );

    GLTexImage (
      TexFormat    format,
      u32          width,
      u32          height,
      u32          layer_count,
      u32          level_count,
      TexImageWrap wrap,
      bool         min_filter,
      bool         mag_filter
    );

    ~GLTexImage ();
    
    void bind (u32 unit)
    {
      if (!glIsTexture (name))
      {
        unbind (unit);
        return;
      }

      glActiveTexture (GL_TEXTURE0 + unit);
      check_gl ("glActiveTexture");

      glBindTexture (target, name);
      check_gl ("glBindTexture");
    }

    static void unbind (u32 unit)
    {
      glActiveTexture (GL_TEXTURE0 + unit);
      check_gl ("glActiveTexture");
      
      glBindTexture (GL_TEXTURE_2D, 0);
      check_gl ("glBindTexture");
    }

  };

}

#endif
