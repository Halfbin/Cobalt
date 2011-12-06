//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLTEXIMAGE
#define CO_GLRENDERER_H_GLTEXIMAGE

// Implements
#include <Co/IxTexImage.hpp>

// Uses
#include <Co/IxRenderContext.hpp>

#include "GL.hpp"

namespace Co
{
  class GLTexImage :
    public IxTexImage
  {
    u32 name,
        target;

    virtual void destroy ();

  protected:
    virtual bool load_map (u32 sub_image, u32 level, const void* data, TexFormat format, u32 width, u32 height, uptr size = 0);
    ~GLTexImage ();
    
  public:
    GLTexImage (uint level_count, TexImageWrap wrap, bool filter, TexImageType type);

    void bind (u32 unit)
    {
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
