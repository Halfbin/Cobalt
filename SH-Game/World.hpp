//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_WORLD
#define SH_H_WORLD

// Implements
#include <Co/Entity.hpp>

// Uses
#include <Co/PropMap.hpp>
#include <Co/Texture.hpp>

#include <Rk/Types.hpp>

#include "Chunk.hpp"

namespace SH
{
  //
  // = World ===========================================================================================================
  //
  class World :
    public Co::Entity
  {
    enum
    {
      stage_dim    = 20,
      stage_chunks = stage_dim * stage_dim * stage_dim,
    };

    /*enum : u64
    {
      max_kbytes = world_chunks * Chunk::max_kbytes,
      max_mbytes = max_kbytes >> 10
    };*/

    // Parameters
    u32 seed;

    // State
    Chunk::Ptr stage [stage_dim][stage_dim][stage_dim];
    uint       slice;
    float      last_slice;

    static Co::Texture::Ptr texture, sky_tex;

    void do_slice ();

    virtual void tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta);

    virtual void render (Co::Frame& frame, float alpha);

    World (Co::WorkQueue& queue);

  public:
    static Ptr create (Co::WorkQueue& queue, const Co::PropMap* props)
    {
      return queue.gc_attach (new World (queue));
    }

    const Chunk::Ptr& chunk_at (v3i cv);
    Block             block_at (v3i bv);

  };

}

#endif
