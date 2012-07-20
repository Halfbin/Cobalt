//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "ChunkMesh.hpp"

namespace SH
{
  struct BlockTCoords
  {
    static const u8 t;

    v2u8 top, sides, bottom;

    BlockTCoords () { }

    BlockTCoords (u8 ts, u8 tt, u8 ss, u8 st, u8 bs, u8 bt) :
      top    (t * v2u8 (ts, tt)),
      sides  (t * v2u8 (ss, st)),
      bottom (t * v2u8 (bs, bt))
    { }
    
  };

  const u8 BlockTCoords::t = 16;

  class BlockTCoordsSet
  {
    BlockTCoords tcoords [256];

  public:
    BlockTCoordsSet ()
    {
      tcoords [blocktype_air  ] = BlockTCoords (0, 0, 0, 0, 0, 0);
      tcoords [blocktype_soil ] = BlockTCoords (2, 0, 2, 0, 2, 0);
      tcoords [blocktype_grass] = BlockTCoords (0, 0, 3, 0, 2, 0);
      tcoords [blocktype_stone] = BlockTCoords (1, 0, 1, 0, 1, 0);
    }

    const BlockTCoords& operator [] (u8 index) const
    {
      return tcoords [index];
    }

  } block_tcoords;

  static const v3u8
    t_light (255, 255, 255),
    f_light (208, 208, 208),
    r_light (160, 160, 160),
    l_light (160, 160, 160),
    b_light (112, 112, 112),
    u_light ( 64,  64,  64);

  struct Vertex
  {
    u8 x, y, z, w;
    u8 s, t, p, q;
    u8 r, g, b, a;
    u8 i, j, k, m;

    Vertex ()
    { }

    Vertex (u8 x, u8 y, u8 z, u8 s, u8 t, v3u8 rgb) :
      x (x), y (y), z (z),
      s (s), t (t),
      r (rgb.x), g (rgb.y), b (rgb.z)
    { }
    
  };

  enum
  {
    vertex_size = sizeof (Vertex),
    max_vertex_bytes = chunk_max_vertices * vertex_size,
    max_index_bytes  = chunk_max_indices  * 2
  };

  //
  // = ChunkMesh =======================================================================================================
  //
  ChunkMesh::ChunkMesh () :
    index_count (0)
  { }
  
  void ChunkMesh::init (Co::WorkQueue& queue, Co::RenderContext& rc)
  {
    vertex_buffer = rc.create_stream (queue, max_vertex_bytes / 4),
    index_buffer  = rc.create_stream (queue, max_index_bytes  / 4);
    
    static const Co::GeomAttrib attribs [3] = {
      { Co::attrib_position, Co::attrib_u8,  16, 0 },
      { Co::attrib_tcoords,  Co::attrib_u8n, 16, 4 },
      { Co::attrib_colour,   Co::attrib_u8n, 16, 8 },
    };

    compilation = rc.create_compilation (queue, attribs, vertex_buffer, index_buffer, Co::index_u16);
  }
  
  static inline uint bit_count (u16 word)
  {
    #if 1
      return (uint) __popcnt16 (short (word));
    #else
      u16 count = word;
      count = ((count >> 1) & 0x5555) + (count & 0x5555);
      count = ((count >> 2) & 0x3333) + (count & 0x3333);
      count = ((count >> 4) & 0x0f0f) + (count & 0x0f0f);
      count = ((count >> 8) & 0x00ff) + (count & 0x00ff);
      return count;
    #endif
  }

  static inline u16 rotr (u16 word, u8 dist)
  {
    #if 1
      return (u16) _rotr16 (ushort (word), uchar (dist));
    #else
      return (word >> dist) | (word << (16 - dist));
    #endif
  }

  uint ChunkMesh::count_faces (const Chunk& chunk, const Chunk* neighbours [6])
  {
    // Now we find transitions in each direction
    uint face_count = 0;

    // Vertical
    for (int x = 0; x != chunk_dim; x++)
    {
      for (int y = 0; y != chunk_dim; y++)
      {
        u16 bits = chunk.opacity [x][y];
        u16 trans = (bits ^ rotr (bits, 1)) & 0x7fff;
        face_count += bit_count (trans);
      }
    }

    // Left-Right
    for (int x = 0; x != chunk_dim; x++)
    {
      for (int z = 0; z != chunk_dim; z++)
      {
        u16 bits = 0;
        for (int y = 0; y != chunk_dim; y++)
          bits |= u16 (chunk.opaque (v3i (x, y, z))) << y;
        u16 trans = (bits ^ rotr (bits, 1)) & 0x7fff;
        face_count += bit_count (trans);
      }
    }

    // Front-Back
    for (int y = 0; y != chunk_dim; y++)
    {
      for (int z = 0; z != chunk_dim; z++)
      {
        u16 bits = 0;
        for (int x = 0; x != chunk_dim; x++)
          bits |= u16 (chunk.opaque (v3i (x, y, z))) << x;
        u16 trans = (bits ^ rotr (bits, 1)) & 0x7fff;
        face_count += bit_count (trans);
      }
    }

    // Finally add transitions between this chunk and the next
    // This gets ugly
    auto front = neighbours [0];
    if (front)
    {
      for (int y = 0; y != chunk_dim; y++)
      {
        for (int z = 0; z != chunk_dim; z++)
        {
          if (!chunk.blocks [chunk_dim - 1][y][z].empty () && front -> blocks [0][y][z].empty ())
            face_count++;
        }
      }
    }

    auto back = neighbours [1];
    if (back)
    {
      for (int y = 0; y != chunk_dim; y++)
      {
        for (int z = 0; z != chunk_dim; z++)
        {
          if (!chunk.blocks [0][y][z].empty () && back -> blocks [chunk_dim - 1][y][z].empty ())
            face_count++;
        }
      }
    }

    auto left = neighbours [2];
    if (left)
    {
      for (int x = 0; x != chunk_dim; x++)
      {
        for (int z = 0; z != chunk_dim; z++)
        {
          if (!chunk.blocks [x][chunk_dim - 1][z].empty () && left -> blocks [x][0][z].empty ())
            face_count++;
        }
      }
    }

    auto right = neighbours [3];
    if (right)
    {
      for (int x = 0; x != chunk_dim; x++)
      {
        for (int z = 0; z != chunk_dim; z++)
        {
          if (!chunk.blocks [x][0][z].empty () && right -> blocks [x][chunk_dim - 1][z].empty ())
            face_count++;
        }
      }
    }

    auto top = neighbours [4];
    if (top)
    {
      for (int x = 0; x != chunk_dim; x++)
      {
        for (int y = 0; y != chunk_dim; y++)
        {
          if (!chunk.blocks [x][y][chunk_dim - 1].empty () && top -> blocks [x][y][0].empty ())
            face_count++;
        }
      }
    }

    auto bottom = neighbours [5];
    if (bottom)
    {
      for (int x = 0; x != chunk_dim; x++)
      {
        for (int y = 0; y != chunk_dim; y++)
        {
          if (!chunk.blocks [x][y][0].empty () && bottom -> blocks [x][y][chunk_dim - 1].empty ())
            face_count++;
        }
      }
    }

    return face_count;
  }

  void ChunkMesh::regen (const Chunk& chunk, const Chunk* neighbours [6])
  {
    log () << "- Regenning chunk (" << chunk.cpos.x << ", " << chunk.cpos.y << ", " << chunk.cpos.z << ")\n";

    uint face_count = count_faces (chunk, neighbours);
    index_count = face_count * 6;

    if (face_count == 0)
      return;

    // Only map as much as we need
    uint vertex_count = face_count * 4;
    auto vertices = (Vertex*) vertex_buffer -> begin (vertex_count * sizeof (Vertex));
    if (!vertices)
    {
      log () << "! whoopsee reserving vertices\n";
      return;
    }

    uint check_face_count = 0;
    
    // Inner
    for (auto b = iteration (v3i (0, 0, 0), chunk_extent); b; b.advance ())
    {
      const auto block = chunk.at (b);

      if (block.empty ())
        continue;

      auto tcoords = block_tcoords [block.type];
      const i8 side_s = tcoords.sides.x,
               side_t = tcoords.sides.y,
               top_s  = tcoords.top.x,
               top_t  = tcoords.top.y,
               bot_s  = tcoords.bottom.x,
               bot_t  = tcoords.bottom.y,
               tg     = BlockTCoords::t;

      if (b.x < chunk_dim - 1 ? chunk.at (b + v3i (1, 0, 0)).empty () : !neighbours [0] -> opaque (v3i (0, b.y, b.z)))
      {
        // front
        *vertices++ = Vertex (b.x + 1, b.y + 0, b.z + 1, side_s +  0, side_t +  0, f_light);
        *vertices++ = Vertex (b.x + 1, b.y + 0, b.z + 0, side_s +  0, side_t + tg, f_light);
        *vertices++ = Vertex (b.x + 1, b.y + 1, b.z + 1, side_s + tg, side_t +  0, f_light);
        *vertices++ = Vertex (b.x + 1, b.y + 1, b.z + 0, side_s + tg, side_t + tg, f_light);
        check_face_count++;
      }

      if (b.x > 0 ? chunk.at (b + v3i (-1, 0, 0)).empty () : !neighbours [1] -> opaque (v3i (chunk_dim - 1, b.y, b.z)))
      {
        // back
        *vertices++ = Vertex (b.x + 0, b.y + 1, b.z + 1, side_s +  0, side_t +  0, b_light);
        *vertices++ = Vertex (b.x + 0, b.y + 1, b.z + 0, side_s +  0, side_t + tg, b_light);
        *vertices++ = Vertex (b.x + 0, b.y + 0, b.z + 1, side_s + tg, side_t +  0, b_light);
        *vertices++ = Vertex (b.x + 0, b.y + 0, b.z + 0, side_s + tg, side_t + tg, b_light);
        check_face_count++;
      }
      
      if (b.y < chunk_dim - 1 ? chunk.at (b + v3i (0, 1, 0)).empty () : !neighbours [2] -> opaque (v3i (b.x, 0, b.z)))
      {
        // left
        *vertices++ = Vertex (b.x + 1, b.y + 1, b.z + 1, side_s +  0, side_t +  0, l_light);
        *vertices++ = Vertex (b.x + 1, b.y + 1, b.z + 0, side_s +  0, side_t + tg, l_light);
        *vertices++ = Vertex (b.x + 0, b.y + 1, b.z + 1, side_s + tg, side_t +  0, l_light);
        *vertices++ = Vertex (b.x + 0, b.y + 1, b.z + 0, side_s + tg, side_t + tg, l_light);
        check_face_count++;
      }
      
      if (b.y > 0 ? chunk.at (b + v3i (0, -1, 0)).empty () : !neighbours [3] -> opaque (v3i (b.x, chunk_dim - 1, b.z)))
      {
        // right
        *vertices++ = Vertex (b.x + 0, b.y + 0, b.z + 1, side_s +  0, side_t +  0, r_light);
        *vertices++ = Vertex (b.x + 0, b.y + 0, b.z + 0, side_s +  0, side_t + tg, r_light);
        *vertices++ = Vertex (b.x + 1, b.y + 0, b.z + 1, side_s + tg, side_t +  0, r_light);
        *vertices++ = Vertex (b.x + 1, b.y + 0, b.z + 0, side_s + tg, side_t + tg, r_light);
        check_face_count++;
      }
      
      if (b.z < chunk_dim - 1 ? chunk.at (b + v3i (0, 0, 1)).empty () : !neighbours [4] -> opaque (v3i (b.x, b.y, 0)))
      {
        // top
        *vertices++ = Vertex (b.x + 1, b.y + 1, b.z + 1, top_s +  0, top_t +  0, t_light);
        *vertices++ = Vertex (b.x + 0, b.y + 1, b.z + 1, top_s +  0, top_t + tg, t_light);
        *vertices++ = Vertex (b.x + 1, b.y + 0, b.z + 1, top_s + tg, top_t +  0, t_light);
        *vertices++ = Vertex (b.x + 0, b.y + 0, b.z + 1, top_s + tg, top_t + tg, t_light);
        check_face_count++;
      }
      
      if (b.z > 0 ? chunk.at (b + v3i (0, 0, -1)).empty () : !neighbours [5] -> opaque (v3i (b.x, b.y, chunk_dim - 1)))
      {
        // bottom
        *vertices++ = Vertex (b.x + 0, b.y + 1, b.z + 0, bot_s +  0, bot_t +  0, u_light);
        *vertices++ = Vertex (b.x + 1, b.y + 1, b.z + 0, bot_s +  0, bot_t + tg, u_light);
        *vertices++ = Vertex (b.x + 0, b.y + 0, b.z + 0, bot_s + tg, bot_t +  0, u_light);
        *vertices++ = Vertex (b.x + 1, b.y + 0, b.z + 0, bot_s + tg, bot_t + tg, u_light);
        check_face_count++;
      }
    }

    assert (check_face_count == face_count);

    bool ok = vertex_buffer -> commit (vertex_count * sizeof (Vertex));
    if (!ok)
    {
      log () << "! whoopsee commiting vertices\n";
      return;
    }

    auto indices = (u16*) index_buffer -> begin (index_count * 2);
    if (!indices)
    {
      log () << "! whoopsee reserving indices\n";
      return;
    }

    for (uint i = 0; i != vertex_count; i += 4)
    {
      *indices++ = i + 0;
      *indices++ = i + 1;
      *indices++ = i + 2;
      *indices++ = i + 1;
      *indices++ = i + 3;
      *indices++ = i + 2;
    }

    ok = index_buffer -> commit (index_count * 2);
    if (!ok)
    {
      log () << "! whoopsee commiting indices\n";
      return;
    }
  }

  void ChunkMesh::draw (Co::Frame& frame, v3i bpos, const Co::Material& mat)
  {
    if (index_count == 0)
      return;

    frame.begin_point_geom (
      compilation,
      Co::Spatial (bpos, nil)
    );

    Co::Mesh mesh (Co::prim_triangles, 0, 0, 0, index_count);

    frame.add_meshes    (&mesh, 1);
    frame.add_materials (&mat,  1);

    frame.end ();
  }

}
