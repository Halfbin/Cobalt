//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_GEOMFILE
#define CO_H_GEOMFILE

#include <Rk/Types.hpp>

namespace Co
{
  // Format is chunked
  // Magic: "CO  GEOM"
  // chunks 4-byte aligned

  // HEAD: Header; mandatory; first
  struct GeomHeader
  {
    u32 version,
        index_count,
        vertex_count,
        frame_count;
    u16 mesh_count,
        seq_count;
  };
  static_assert (sizeof (GeomHeader) == 16, "GeomHeader miscompiled");

  // IDCS: Indices; mandatory
  // u32 * index_count

  // VRTS: Vertices: mandatory
  struct GeomVertex
  {
    f32 x, y, z, // Position
        i, j, k, // Normal
        u, v;    // Texture Coords
  };

  // MSHS: Meshes: mandatory
  // u32 * mesh_count
  // If m [n] is the nth entry, and is the index of the starting index of the nth mesh
  // The nth mesh contains m [n+1] - m [n] consecutive indices
  
  // MMAP: Material map: optional
  // u32 * mesh_count
  // The nth entry is the index of the material name corresponding to the nth mesh

  // MTNS: Material names: optional
  // A list of null-terminated strings containing material names

  // VWTS: Vertex Weights

  // 
}

#endif
