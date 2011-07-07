//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLTEXTURE
#define CO_GLRENDERER_H_GLTEXTURE

#include <Co/IxTexture.hpp>

namespace Co
{
  class GLTexture :
    public IxTexture
  {
    u32 name;

    virtual void load_map (uint level, const void* data, TexFormat format, uint width, uint height, uptr size);

    ~GLTexture ();
    virtual void destroy ();

  public:
    GLTexture (uint level_count, bool wrap);

  };

}

#endif
