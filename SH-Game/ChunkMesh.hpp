//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_CHUNKMESH
#define SH_H_CHUNKMESH

#include <Co/GeomCompilation.hpp>
#include <Co/RenderContext.hpp>
#include <Co/GeomBuffer.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/Frame.hpp>

#include <Rk/Vector3.hpp>
#include <Rk/Types.hpp>

#include "Chunk.hpp"

namespace SH
{
  class ChunkMesh
  {
    Co::GeomCompilation::Ptr compilation;
    Co::StreamBuffer::Ptr    vertex_buffer,
                             index_buffer;
    uint                     index_count;

    uint count_faces (const Chunk& chunk, const Chunk* neighbours [6]);

  public:
    ChunkMesh ();

    void init (Co::WorkQueue& queue, Co::RenderContext& rc);
    
    void regen (const Chunk& chunk, const Chunk* neighbours [6]);
    void draw  (Co::Frame& frame, v3i bpos, const Co::Material& mat);

  };

}

#endif
