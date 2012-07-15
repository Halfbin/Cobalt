//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "Chunk.hpp"

// Uses
#include "World.hpp"
#include "Common.hpp"
#include "Noise.hpp"

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
    max_vertex_bytes = Chunk::max_vertices * vertex_size,
    max_index_bytes  = Chunk::max_indices  * 2
  };

  //
  // = Chunk ===========================================================================================================
  //
  void Chunk::init (Co::WorkQueue& queue, Co::RenderContext& rc)
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

  void Chunk::slice (uint limit)
  {
    for (int x = 0; x != dim; x++)
    {
      for (int y = 0; y != dim; y++)
        blocks [x][y][limit - bpos.z].type = blocktype_air;
    }

    dirty = true;
  }

  void Chunk::draw (Co::Frame& frame, const Co::Material& mat)
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

  void Chunk::generate (Ptr self, Co::WorkQueue& queue, u32 seed)
  {
    if (loaded || loading)
      return;

    loading = true;
    queue.queue_load (
      [self, seed] (Co::WorkQueue& queue, Co::RenderContext& rc, Co::Filesystem&)
      {
        self -> init (queue, rc);
        self -> generate_impl (seed);

        // Fuck you c++
        auto self_copy = self;
        queue.queue_completion (
          [self_copy]
          {
            self_copy -> loaded = true;
            self_copy -> dirty  = true;
          }
        );
      }
    );
  }

  //
  // = Chunk generation ================================================================================================
  //
  void Chunk::generate_impl (u32 seed)
  {
    // Land
    if (bpos.z > 85)
    {
      for (int x = 0; x != dim; x++)
      {
        for (int y = 0; y != dim; y++)
        {
          for (int z = 0; z != dim; z++)
            blocks [x][y][z].type = blocktype_air;
        }
      }
    }
    else if (bpos.z > 25)
    {
      for (int x = 0; x != dim; x++)
      {
        for (int y = 0; y != dim; y++)
        {
          v2f noise_pos = cpos.xy () + v2f (x, y) * (1.0f / dim);
          int level = int (64.0f + 22.0f * noise_perlin_harmonic (noise_pos, seed, 0.3f, 3, 0.25f));
          level -= bpos.z;

          int depth = int (8.0f + 10.0f * noise_perlin_harmonic (noise_pos, seed + 20, 0.3f, 3, 0.25f));
          depth = level - depth;

          int z = 0;

          while (z < std::min (depth, int (dim)))
            blocks [x][y][z++].type = blocktype_stone;

          while (z < std::min (level, int (dim)))
            blocks [x][y][z++].type = blocktype_soil;

          if (level >= 0 && level < dim)
            blocks [x][y][z++].type = blocktype_grass;

          while (z < dim)
            blocks [x][y][z++].type = blocktype_air;
        }
      }
    }
    else
    {
      for (int x = 0; x != dim; x++)
      {
        for (int y = 0; y != dim; y++)
        {
          for (int z = 0; z != dim; z++)
            blocks [x][y][z].type = blocktype_stone;
        }
      }
    }

    // Caves
    if (bpos.z < 65)
    {
      for (int x = 0; x != dim; x++)
      {
        for (int y = 0; y != dim; y++)
        {
          for (int z = 0; z != dim; z++)
          {
            v3f noise_pos = cpos + v3f (x, y, z) * (1.0f / dim);
            bool cut = -0.2f > noise_perlin_harmonic (noise_pos, seed, 0.5f, 3, 0.8f);

            if (cut)
              blocks [x][y][z].type = blocktype_air;
          }
        }
      }
    }
  }

  //
  // = regen_mesh ======================================================================================================
  //
  inline uint bit_count (u16 word)
  {
    return (uint) __popcnt16 (short (word));
    #if 0
      u16 count = word;
      count = ((count >> 1) & 0x5555) + (count & 0x5555);
      count = ((count >> 2) & 0x3333) + (count & 0x3333);
      count = ((count >> 4) & 0x0f0f) + (count & 0x0f0f);
      count = ((count >> 8) & 0x00ff) + (count & 0x00ff);
      return count;
    #endif
  }

  /*struct Test
  {
    Test ()
    {
      for (uint i = 0; i != 0x10000; i++)
      {
        const u16 bits = i & 0xffff;

        uint transitions_a = 0;
        bool prev = (bits & 1) == 1;
        for (uint j = 1; j != 16; j++)
        {
          bool cur = ((bits >> j) & 1) == 1;
          if (cur != prev)
          {
            prev = cur;
            transitions_a++;
          }
        }

        u16 trans = (bits ^ _rotr16 (bits, 1)) & 0x7fff;//((bits >> 1) | 0x8000);
        uint transitions_b = bit_count (trans);

        assert (transitions_a == transitions_b);
      }
    }

  } test;*/

  void Chunk::regen_mesh (World& world)
  {
    log () << "- Regenning chunk (" << cpos.x << ", " << cpos.y << ", " << cpos.z << ")\n";

    // Figure out how many faces will be generated
    u16 transp [3 * dim * dim];

    // Fill transp with 1s for transparent blocks, 0s for opaque ones
    int i = 0;

    for (int y = 0; y != dim; y++)
    {
      for (int z = 0; z != dim; z++, i++)
      {
        transp [i] = 0;
        for (int x = 0; x != dim; x++)
          transp [i] |= u16 (blocks [x][y][z].empty ()) << x;
      }
    }

    for (int x = 0; x != dim; x++)
    {
      for (int z = 0; z != dim; z++, i++)
      {
        transp [i] = 0;
        for (int y = 0; y != dim; y++)
          transp [i] |= u16 (blocks [x][y][z].empty ()) << y;
      }
    }

    for (int x = 0; x != dim; x++)
    {
      for (int y = 0; y != dim; y++, i++)
      {
        transp [i] = 0;
        for (int z = 0; z != dim; z++)
          transp [i] |= u16 (blocks [x][y][z].empty ()) << z;
      }
    }

    // Now we find transitions in each direction
    uint face_count = 0;

    for (uint j = 0; j != i; j++)
    {
      u16 bits = transp [j];
      u16 trans = (bits ^ _rotr16 (bits, 1)) & 0x7fff;
      // If there are n transitions in bits, then trans contains n 1-bits
      face_count += bit_count (trans);
    }

    // Finally add transitions between this chunk and the next
    // This gets ugly
    auto front = world.chunk_at (cpos + v3i (1, 0, 0));
    for (int y = 0; y != dim; y++)
    {
      for (int z = 0; z != dim; z++)
      {
        if (!blocks [dim - 1][y][z].empty () && front -> blocks [0][y][z].empty ())
          face_count++;
      }
    }

    auto back = world.chunk_at (cpos + v3i (-1, 0, 0));
    for (int y = 0; y != dim; y++)
    {
      for (int z = 0; z != dim; z++)
      {
        if (!blocks [0][y][z].empty () && back -> blocks [dim - 1][y][z].empty ())
          face_count++;
      }
    }

    auto left = world.chunk_at (cpos + v3i (0, 1, 0));
    for (int x = 0; x != dim; x++)
    {
      for (int z = 0; z != dim; z++)
      {
        if (!blocks [x][dim - 1][z].empty () && left -> blocks [x][0][z].empty ())
          face_count++;
      }
    }

    auto right = world.chunk_at (cpos + v3i (0, -1, 0));
    for (int x = 0; x != dim; x++)
    {
      for (int z = 0; z != dim; z++)
      {
        if (!blocks [x][0][z].empty () && right -> blocks [x][dim - 1][z].empty ())
          face_count++;
      }
    }

    auto top = world.chunk_at (cpos + v3i (0, 0, 1));
    for (int x = 0; x != dim; x++)
    {
      for (int y = 0; y != dim; y++)
      {
        if (!blocks [x][y][dim - 1].empty () && top -> blocks [x][y][0].empty ())
          face_count++;
      }
    }

    auto bottom = world.chunk_at (cpos + v3i (0, 0, -1));
    for (int x = 0; x != dim; x++)
    {
      for (int y = 0; y != dim; y++)
      {
        if (!blocks [x][y][0].empty () && bottom -> blocks [x][y][dim - 1].empty ())
          face_count++;
      }
    }
    
    if (face_count == 0)
    {
      dirty = false;
      return;
    }

    // Only map as much as we need
    uint vertex_count = face_count * 4;
    auto vertices = (Vertex*) vertex_buffer -> begin (vertex_count * sizeof (Vertex));
    if (!vertices)
      return;

    index_count = face_count * 6;
    auto indices = (u16*) index_buffer -> begin (index_count * 2);
    if (!indices)
    {
      vertex_buffer -> commit (0);
      return;
    }

    uint check_face_count = 0;

    for (int x = 0; x != dim; x++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int z = 0; z != dim; z++)
        {
          const auto block = blocks [x][y][z];

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
          
          if (world.block_at (bpos + v3i (x + 1, y, z)).empty ())
          {
            // front
            *vertices++ = Vertex (x + 1, y + 0, z + 1, side_s +  0, side_t +  0, f_light);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, side_s +  0, side_t + tg, f_light);
            *vertices++ = Vertex (x + 1, y + 1, z + 1, side_s + tg, side_t +  0, f_light);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, side_s + tg, side_t + tg, f_light);
            check_face_count++;
          }

          if (world.block_at (bpos + v3i (x - 1, y, z)).empty ())
          {
            // back
            *vertices++ = Vertex (x + 0, y + 1, z + 1, side_s +  0, side_t +  0, b_light);
            *vertices++ = Vertex (x + 0, y + 1, z + 0, side_s +  0, side_t + tg, b_light);
            *vertices++ = Vertex (x + 0, y + 0, z + 1, side_s + tg, side_t +  0, b_light);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, side_s + tg, side_t + tg, b_light);
            check_face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y + 1, z)).empty ())
          {
            // left
            *vertices++ = Vertex (x + 1, y + 1, z + 1, side_s +  0, side_t +  0, l_light);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, side_s +  0, side_t + tg, l_light);
            *vertices++ = Vertex (x + 0, y + 1, z + 1, side_s + tg, side_t +  0, l_light);
            *vertices++ = Vertex (x + 0, y + 1, z + 0, side_s + tg, side_t + tg, l_light);
            check_face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y - 1, z)).empty ())
          {
            // right
            *vertices++ = Vertex (x + 0, y + 0, z + 1, side_s +  0, side_t +  0, r_light);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, side_s +  0, side_t + tg, r_light);
            *vertices++ = Vertex (x + 1, y + 0, z + 1, side_s + tg, side_t +  0, r_light);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, side_s + tg, side_t + tg, r_light);
            check_face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y, z + 1)).empty ())
          {
            // top
            *vertices++ = Vertex (x + 1, y + 1, z + 1, top_s +  0, top_t +  0, t_light);
            *vertices++ = Vertex (x + 0, y + 1, z + 1, top_s +  0, top_t + tg, t_light);
            *vertices++ = Vertex (x + 1, y + 0, z + 1, top_s + tg, top_t +  0, t_light);
            *vertices++ = Vertex (x + 0, y + 0, z + 1, top_s + tg, top_t + tg, t_light);
            check_face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y, z - 1)).empty ())
          {
            // bottom
            *vertices++ = Vertex (x + 0, y + 1, z + 0, bot_s +  0, bot_t +  0, u_light);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, bot_s +  0, bot_t + tg, u_light);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, bot_s + tg, bot_t +  0, u_light);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, bot_s + tg, bot_t + tg, u_light);
            check_face_count++;
          }
        } // z
      } // y
    } // x

    assert (check_face_count == face_count);

    for (uint i = 0; i != vertex_count; i += 4)
    {
      *indices++ = i + 0;
      *indices++ = i + 1;
      *indices++ = i + 2;
      *indices++ = i + 1;
      *indices++ = i + 3;
      *indices++ = i + 2;
    }

    bool ok = vertex_buffer -> commit (vertex_count * sizeof (Vertex));
    if (!ok)
      return;

    ok = index_buffer -> commit (index_count * 2);
    if (!ok)
      return;
    
    dirty = false;
  }

} // namespace SH
