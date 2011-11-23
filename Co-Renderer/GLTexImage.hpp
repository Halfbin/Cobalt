//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLTEXIMAGE
#define CO_GLRENDERER_H_GLTEXIMAGE

// Implements
#include <Co/IxTexImage.hpp>

// Uses
#include "GL.hpp"

namespace Co
{
  enum TexUnit
  {
    texunit_diffuse  = 0,
    texunit_specular = 1,
    texunit_emission = 2,
    texunit_exponent = 3,
    texunit_normal   = 4
  };

  class GLTexImage :
    public IxTexImage
  {
    u32 name;

    virtual void destroy ();

  protected:
    virtual void load_map (uint level, const void* data, TexFormat format, uint width, uint height, uptr size = 0);
    ~GLTexImage ();
    
  public:
    GLTexImage (uint level_count, bool wrap);

    void bind (TexUnit unit)
    {
      glActiveTexture (GL_TEXTURE0 + unit);
      check_gl ("glActiveTexture");

      glBindTexture (GL_TEXTURE_2D, name);
      check_gl ("glBindTexture");
    }

  };

}

#endif
