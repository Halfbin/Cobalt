//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_CHUNK
#define SH_H_CHUNK

// Uses
#include <Co/GeomCompilation.hpp>
#include <Co/RenderContext.hpp>
#include <Co/GeomBuffer.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/Frame.hpp>

#include <Rk/Vector3.hpp>
#include <Rk/Types.hpp>

#include <memory>

#include "Common.hpp"
#include "Block.hpp"

namespace SH
{
  class Chunk
  {
    float gen_time;

    void generate_impl (u32 seed);

  public:
    typedef std::shared_ptr <Chunk> Ptr;

    // Data
    Block blocks [chunk_dim][chunk_dim][chunk_dim];

    // Caches
    u16 x_opacity  [chunk_dim][chunk_dim],
        y_opacity  [chunk_dim][chunk_dim],
        z_opacity  [chunk_dim][chunk_dim];
    //u16 solidity [chunk_dim][chunk_dim];
    uint internal_face_count;

    // State
    bool loaded,
         dirty;

    // Identity
    const v3i cpos,
              bpos;

    Chunk (v3i cpos);
    ~Chunk ();

    static uptr high_mem ();

    float get_gen_time () const;

    Block& at (v3i bv)
    {
      return blocks [bv.x][bv.y][bv.z];
    }

    __forceinline Block at (v3i bv) const
    {
      return blocks [bv.x][bv.y][bv.z];
    }

    bool opaque (v3i bv) const
    {
      return (z_opacity [bv.x][bv.y] >> bv.z) & 1;
    }

    /*bool solid (v3i bv) const
    {
      return (solidity [bv.x][bv.y] >> bv.z) & 1;
    }*/

    void generate (Ptr self, Co::WorkQueue& queue, u32 seed);
    
    void regen_caches ();

  }; // class Chunk

}

#endif
