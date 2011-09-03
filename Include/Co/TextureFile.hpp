//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_TEXTUREFILE
#define CO_H_TEXTUREFILE

#include <Rk/Types.hpp>

namespace Co
{
  static const char texture_magic [8] = { 'C', 'O', 'T', 'E', 'X', 'T', 'R', '1' };

  struct TextureHeader
  {
    u32 version;
    u16 map_count,
        format,
        width,
        height;
  };
  static_assert (sizeof (TextureHeader) == 12, "TextureHeader miscompiled");
  
}

#endif // RKMATERIAL_TEST_EXE
