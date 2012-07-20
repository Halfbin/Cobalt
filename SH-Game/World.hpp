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

#include "ChunkMesh.hpp"
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
    // Parameters
    u32 seed;

    // State
    typedef std::unordered_map <v3i, Chunk::Ptr> Cache;

    struct StageChunk
    {
      Chunk::Ptr chunk;
      ChunkMesh  mesh;
    };

    StageChunk stage [stage_dim][stage_dim][stage_dim];
    v3i        stage_base;
    //v3i        view_cur_cpos;
    
    Cache cache;
    
    Co::Texture::Ptr texture,
                     sky_tex;

    StageChunk& stage_at (v3i cv);

    Chunk::Ptr load_chunk (Co::WorkQueue& queue, v3i cpos);

    virtual void tick   (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta);
    virtual void render (Co::Frame& frame, float alpha);

  public:
    World (Co::WorkQueue& queue, Co::RenderContext& rc, const Co::PropMap* props);

  };

}

#endif
