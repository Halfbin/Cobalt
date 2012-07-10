//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_BLOCK
#define SH_H_BLOCK

// Uses
#include <Rk/Types.hpp>

namespace SH
{
  enum BlockType :
    u8
  {
    blocktype_air   = 0x00,
    blocktype_soil  = 0x01,
    blocktype_grass = 0x02,

    blocktype_void  = 0xff
  };
  
  class Block
  {
  public:
    BlockType type;

    Block ()
    { }

    Block (BlockType new_type) :
      type (new_type)
    { }
    
    bool empty () const
    {
      return type == blocktype_air;
    }

  };

}

#endif
