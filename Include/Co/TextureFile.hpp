//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_TEXTUREFILE
#define CO_H_TEXTUREFILE

#include <Rk/Types.hpp>

namespace Co
{
  enum TexFormat :
    u8
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

    texformat_r8,
    texformat_a8,

    texformat_count
  };
  
  enum TexFlags :
    u8
  {
    texflag_cube_map = 0x01,

    texflag_mask = 0x01
  };
  
  enum TexFace :
    u16
  {
    texface_front  = 0,
    texface_back   = 1,
    texface_left   = 2,
    texface_right  = 3,
    texface_top    = 4,
    texface_bottom = 5
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
    u32       version;   // 4
    u8        flags;     // 5
    u8        map_count; // 6
    TexFormat format;    // 8
    u16       width,     // 10
              height;    // 12
  };
  static_assert (sizeof (TextureHeader) == 12, "TextureHeader miscompiled");
  
}

#endif // RKMATERIAL_TEST_EXE
