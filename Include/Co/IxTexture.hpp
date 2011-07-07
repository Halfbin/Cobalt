//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXTEXTURE
#define CO_H_IXTEXTURE

#include <Rk/Types.hpp>
#include <Rk/IxUnique.hpp>

namespace Co
{
  enum TexFormat : uint
  {
    tex_rgb565 = 0,
    tex_bgr565,
    tex_rgb888,
    tex_bgr888,
    tex_rgba8888,
    tex_bgra8888,

    tex_dxt1,
    tex_dxt1a,
    tex_dxt3,
    tex_dxt5,
    tex_dxtn,

    texformat_count
  };
  
  class IxTexture :
    public Rk::IxUnique
  {
  public:
    virtual void load_map (uint level, const void* data, TexFormat format, uint width, uint height, uptr size = 0) = 0;

  };

} // namespace Co

#endif
