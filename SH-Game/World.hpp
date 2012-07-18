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

#include <unordered_map>

namespace SH
{
  //
  // = World ===========================================================================================================
  //
  class World :
    public Co::Entity
  {
  public:
    static const int
      stage_dim    = 13,
      stage_radius = stage_dim / 2,
      stage_chunks = stage_dim * stage_dim * stage_dim
    ;

  private:
    // Parameters
    u32 seed;

    // State
    typedef std::unordered_map <v3i, Chunk::Ptr> Cache;

    Chunk::Ptr stage [stage_dim][stage_dim][stage_dim];
    Cache      cache;
    v3i        view_cur_cpos;
    
    Chunk::Ptr load_chunk (v3i cpos);

    /*static*/ Co::Texture::Ptr texture, sky_tex;

    virtual void tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta);

    virtual void render (Co::Frame& frame, float alpha);

  public:
    World (Co::WorkQueue& queue, const Co::PropMap* props);

    const Chunk::Ptr& chunk_at (v3i cv);
    Block             block_at (v3i bv);

  };

}

#endif
