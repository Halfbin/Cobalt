//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_TEXTUREFILE
#define CO_H_TEXTUREFILE

#include <Rk/Types.hpp>

namespace Co
{
  enum TexFormat :
    u16
  {
    texformat_rgb565 = 0,
    texformat_bgr565,
    texformat_rgb888,
    texformat_bgr888,
    texformat_rgba8888,
    texformat_bgra8888,

    texformat_dxt1,
    texformat_dxt1a,
    texformat_dxt3,
    texformat_dxt5,
    texformat_dxtn,

    tex_r8,

    texformat_count
  };
  
  static const char* tex_format_strings [texformat_count] = {
    "rgb565",
    "bgr565",
    "rgb888",
    "bgr888",
    "rgba8888",
    "bgra8888",
    "dxt1",
    "dxt1a",
    "dxt3",
    "dxt5",
    "dxtn",
    "r8"
  };

  static const char texture_magic [8] = { 'C', 'O', 'T', 'E', 'X', 'T', 'R', '1' };

  struct TextureHeader
  {
    u32       version;
    u16       map_count;
    TexFormat format;
    u16       width,
              height;
  };
  static_assert (sizeof (TextureHeader) == 12, "TextureHeader miscompiled");
  
}

#endif // RKMATERIAL_TEST_EXE
