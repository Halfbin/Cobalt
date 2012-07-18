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

#include "Block.hpp"

namespace SH
{
  class World;

  class Chunk
  {
    void generate_impl (u32 seed);

  public:
    typedef std::shared_ptr <Chunk> Ptr;

    static const int
      dim          = 16,
      lb_dim       = 4,
      max_faces    = ((dim * dim * dim + 1) / 2) * 6,
      max_vertices = max_faces * 4,
      max_indices  = max_faces * 6
    ;

    // Resources
    Co::GeomCompilation::Ptr compilation;
    Co::StreamBuffer::Ptr    vertex_buffer,
                             index_buffer;

    // State
    Block     blocks [dim][dim][dim];
    uptr      index_count;
    bool      loaded,
              loading,
              dirty;
    v3i       cpos,
              bpos;

    Chunk (v3i cpos) :
      index_count (0),
      loaded  (false),
      loading (false),
      dirty   (false),
      cpos    (cpos),
      bpos    (cpos * dim)
    { }
    
    Block& at (v3i bv)
    {
      return blocks [bv.x][bv.y][bv.z];
    }

    void init        (Co::WorkQueue& queue, Co::RenderContext& rc);
    void generate    (Ptr self, Co::WorkQueue& queue, u32 seed);
    uint count_faces (World& world, v3i stage_cpos);
    void regen_mesh  (World& world, v3i stage_cpos);
    void draw        (Co::Frame& frame, const Co::Material& mat);

    void slice (uint limit);

  }; // class Chunk

}

#endif
