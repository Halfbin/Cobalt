//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_WORLD
#define SH_H_WORLD

// Uses
#include <Co/PropMap.hpp>
#include <Co/Texture.hpp>

#include <Rk/Types.hpp>

#include "ChunkMesh.hpp"
#include "Chunk.hpp"

#include <unordered_map>
#include <vector>

namespace SH
{
  //
  // = World ===========================================================================================================
  //
  class World
  {
    // Parameters
    u32 seed;

    // Cache
    typedef std::unordered_map <v3i, Chunk::Ptr> Cache;
    Cache cache;
    
    // Stage
    struct StageChunk
    {
      Chunk::Ptr chunk;
      ChunkMesh  mesh;
    };

    StageChunk stage [stage_dim][stage_dim][stage_dim];
    v3i        stage_base;

    // Scratch
    struct DrawChunk
    {
      StageChunk* schunk;
      uint        dist;

      DrawChunk (StageChunk* schunk, uint dist) :
        schunk (schunk),
        dist   (dist)
      { }
      
    };

    std::vector <DrawChunk> draw_list;
    
    // Resources
    Co::Texture::Ptr texture,
                     sky_tex;

    StageChunk& stage_at (v3i cv)
    {
      cv = (stage_base + cv) % stage_dim;
      if (cv.x < 0) cv.x += stage_dim;
      if (cv.y < 0) cv.y += stage_dim;
      if (cv.z < 0) cv.z += stage_dim;
      return stage [cv.x][cv.y][cv.z];
    }

    Chunk::Ptr load_chunk (Co::WorkQueue& queue, v3i cpos);

    World (Co::WorkQueue& queue, Co::RenderContext& rc);
    
  public:
    typedef std::shared_ptr <World> Ptr;

    static Ptr create (Co::WorkQueue& queue, Co::RenderContext& rc)
    {
      return Ptr (new World (queue, rc));
    }

    void tick (float time, float step, Co::WorkQueue& queue);
    void render (Co::Frame& frame, float alpha);

    ~World ();

  };

}

#endif
