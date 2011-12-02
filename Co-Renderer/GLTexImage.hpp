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
    virtual void load_map (uint level, const void* data, TexFormat format, uint width, uint height, uptr size = 0);
    ~GLTexImage ();
    
  public:
    GLTexImage (uint level_count, TexImageWrap wrap, TexImageFilter filter, TexImageType type);

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
