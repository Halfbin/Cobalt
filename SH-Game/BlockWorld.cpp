//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Entity.hpp>

// Uses
#include <Co/GeomCompilation.hpp>
#include <Co/RenderContext.hpp>
#include <Co/EntityClass.hpp>
#include <Co/GeomBuffer.hpp>
#include <Co/Profile.hpp>
#include <Co/Frame.hpp>

#include <Rk/AsyncMethod.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Lerp.hpp>

#include <vector>

#include "Common.hpp"
#include "Noise.hpp"

namespace SH
{
  enum BlockType :
    u8
  {
    blocktype_air   = 0x00,
    blocktype_soil  = 0x01,
    blocktype_grass = 0x02,

    blocktype_void  = 0xff
  };
  
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
    }

    const BlockTCoords& operator [] (u8 index) const
    {
      return tcoords [index];
    }

  } block_tcoords;

  class Block
  {
  public:
    BlockType type;

    Block () { }

    Block (BlockType new_type) :
      type (new_type)
    { }
    
    bool empty () const
    {
      return type == blocktype_air;
    }

  };

  struct Vertex
  {
    u8 x, y, z;
    u8 s, t;
    u8 r, g, b;

    Vertex ()
    { }

    Vertex (u8 x, u8 y, u8 z, u8 s, u8 t, u8 r, u8 g, u8 b) :
      x (x), y (y), z (z),
      s (s), t (t),
      r (r), g (g), b (b)
    { }
    
  };

  enum { vertex_size = sizeof (Vertex) };

  //
  // = Chunk ===========================================================================================================
  //
  class BlockWorld;

  class Chunk
  {
    void generate_impl (u32 seed);

  public:
    typedef std::shared_ptr <Chunk> Ptr;

    enum
    {
      dim              = 16,
      max_faces        = ((dim * dim * dim + 1) / 2) * 6,
      max_vertices     = max_faces * 4,
      max_indices      = max_faces * 6,

      max_vertex_bytes = max_vertices * sizeof (Vertex),
      max_index_bytes  = max_indices  * 2,
      max_bytes        = max_vertex_bytes + max_index_bytes,
      max_kbytes       = max_bytes >> 10
    };

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
      bpos    (cpos * uint (dim))
    { }
    
    Block& at (v3i bv)
    {
      return blocks [bv.x][bv.y][bv.z];
    }

    void init (Co::WorkQueue& queue, Co::RenderContext& rc)
    {
      vertex_buffer = rc.create_stream (queue, max_vertex_bytes / 4),
      index_buffer  = rc.create_stream (queue, max_index_bytes  / 4);
      
      static const Co::GeomAttrib attribs [3] = {
        { Co::attrib_position, Co::attrib_u8,  8, 0 },
        { Co::attrib_tcoords,  Co::attrib_u8n, 8, 3 },
        { Co::attrib_colour,   Co::attrib_u8n, 8, 5 },
      };

      compilation = rc.create_compilation (queue, attribs, vertex_buffer, index_buffer, Co::index_u16);
    }

    void regen_mesh (BlockWorld& world);

    void slice (uint limit)
    {
      for (int x = 0; x != dim; x++)
      {
        for (int y = 0; y != dim; y++)
          blocks [x][y][limit - bpos.z].type = blocktype_air;
      }

      dirty = true;
    }

    void draw (Co::Frame& frame, const Co::Material& mat)
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

    void generate (Ptr self, Co::WorkQueue& queue, u32 seed)
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

  }; // class Chunk
  
  //
  // = BlockWorld ======================================================================================================
  //
  class BlockWorld :
    public Co::Entity
  {
    enum
    {
      world_dim    = 16,
      world_chunks = world_dim * world_dim * world_dim,
    };

    enum : u64
    {
      max_kbytes = world_chunks * Chunk::max_kbytes,
      max_mbytes = max_kbytes >> 10
    };

    // Parameters
    u64 seed;

    // State
    Chunk::Ptr stage [world_dim][world_dim][world_dim];
    uint       slice;
    float      last_slice;

    static Co::Texture::Ptr texture, sky_tex;

    void do_slice ()
    {
      for (int x = 0; x != world_dim; x++)
      {
        for (int y = 0; y != world_dim; y++)
          stage [x][y][slice >> 4] -> slice (slice);
      }

      slice--;
    }

    virtual void tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta)
    {
      if (time >= last_slice + 1.0f && slice > 45)
      {
        do_slice ();
        last_slice += 1.0f;
      }

      //Co::Profiler tick ("tick", *log_ptr);

      for (int x = 0; x != world_dim; x++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int z = 0; z != world_dim; z++)
          {
            auto& chunk = chunk_at (v3i (x, y, z));

            if (!chunk -> loaded)
            {
              if (!chunk -> loading)
                chunk -> generate (chunk, queue, seed);
              continue;
            }

            // ...
          }
        }
      }

      //tick.done ();
    }

    virtual void render (Co::Frame& frame, float alpha)
    {
      Co::Material material = nil;
      material.diffuse_tex = texture -> get ();

      bool loaded [world_dim][world_dim][world_dim];

      for (int x = 0; x != world_dim; x++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int z = 0; z != world_dim; z++)
            loaded [x][y][z] = stage [x][y][z] -> loaded;
        }
      }

      for (int x = 1; x != world_dim - 1; x++)
      {
        for (int y = 1; y != world_dim - 1; y++)
        {
          for (int z = 1; z != world_dim - 1; z++)
          {
            auto& chunk = stage [x][y][z];

            bool ok = loaded [x][y][z];
            ok = ok && loaded [x - 1][y][z] && loaded [x + 1][y][z];
            ok = ok && loaded [x][y - 1][z] && loaded [x][y + 1][z];
            ok = ok && loaded [x][y][z - 1] && loaded [x][y][z + 1];
            if (!ok)
              continue;

            if (chunk -> dirty)
              chunk -> regen_mesh (*this);
            
            if (chunk -> index_count > 0)
              chunk -> draw (frame, material);
          }
        }
      }

      frame.set_skybox (
        sky_tex -> get (),
        v3f (0.0f, 0.0f, 0.0f),
        1.0f
      );
    }

    BlockWorld (Co::WorkQueue& queue)
    {
      seed = 10101;

      slice = 80;
      last_slice = 10.0f;

      texture = texture_factory -> create (queue, "blocks.cotexture", false, false, false);
      sky_tex = texture_factory -> create (queue, "title.cotexture",  false, true, true);

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
            stage [x][y][z] = std::make_shared <Chunk> (v3i (x, y, z));
        }
      }
    }

  public:
    static Ptr create (Co::WorkQueue& queue, const Co::PropMap* props)
    {
      return queue.gc_attach (new BlockWorld (queue));
    }

    const Chunk::Ptr& chunk_at (v3i cv)
    {
      if (
        cv.x >= world_dim || cv.x < 0 ||
        cv.y >= world_dim || cv.y < 0 ||
        cv.z >= world_dim || cv.z < 0)
      {
        static const Chunk::Ptr null = nullptr;
        return null;
      }

      return stage [cv.x][cv.y][cv.z];
    }

    Block block_at (v3i bv)
    {
      v3i cv (bv.x >> 4, bv.y >> 4, bv.z >> 4) ;
      bv = v3i (bv.x & 0xf, bv.y & 0xf, bv.z & 0xf);

      if (cv.x >= world_dim || cv.y >= world_dim || cv.z >= world_dim || cv.x < 0 || cv.y < 0 || cv.z < 0)
        return Block (blocktype_void);
      else
        return chunk_at (cv) -> at (bv);
    }

  };

  Co::Texture::Ptr BlockWorld::sky_tex,
                   BlockWorld::texture;

  //
  // = Chunk generation ================================================================================================
  //
  void Chunk::generate_impl (u32 seed)
  {
    for (int x = 0; x != dim; x++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int z = 0; z != dim; z++)
        {
          v2f noise_pos = cpos.xy () + v2f (x, y) * (1.0f / dim);
          float value = 64.0f + 16.0f * noise_perlin_harmonic (noise_pos, seed, 0.3f, 3, 0.25f);
          blocks [x][y][z].type = (z + bpos.z < value) ? blocktype_soil : blocktype_air;
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

  struct Test
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

  } test;

  void Chunk::regen_mesh (BlockWorld& world)
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
            *vertices++ = Vertex (x + 1, y + 0, z + 1, side_s +  0, side_t +  0, 205, 205, 205);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, side_s +  0, side_t + tg, 205, 205, 205);
            *vertices++ = Vertex (x + 1, y + 1, z + 1, side_s + tg, side_t +  0, 205, 205, 205);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, side_s + tg, side_t + tg, 205, 205, 205);
          }

          if (world.block_at (bpos + v3i (x - 1, y, z)).empty ())
          {
            // back
            *vertices++ = Vertex (x + 0, y + 1, z + 1, side_s +  0, side_t +  0, 154, 154, 154);
            *vertices++ = Vertex (x + 0, y + 1, z + 0, side_s +  0, side_t + tg, 154, 154, 154);
            *vertices++ = Vertex (x + 0, y + 0, z + 1, side_s + tg, side_t +  0, 154, 154, 154);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, side_s + tg, side_t + tg, 154, 154, 154);
          }
            
          if (world.block_at (bpos + v3i (x, y + 1, z)).empty ())
          {
            // left
            *vertices++ = Vertex (x + 1, y + 1, z + 1, side_s +  0, side_t +  0, 179, 179, 179);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, side_s +  0, side_t + tg, 179, 179, 179);
            *vertices++ = Vertex (x + 0, y + 1, z + 1, side_s + tg, side_t +  0, 179, 179, 179);
            *vertices++ = Vertex (x + 0, y + 1, z + 0, side_s + tg, side_t + tg, 179, 179, 179);
          }
            
          if (world.block_at (bpos + v3i (x, y - 1, z)).empty ())
          {
            // right
            *vertices++ = Vertex (x + 0, y + 0, z + 1, side_s +  0, side_t +  0, 179, 179, 179);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, side_s +  0, side_t + tg, 179, 179, 179);
            *vertices++ = Vertex (x + 1, y + 0, z + 1, side_s + tg, side_t +  0, 179, 179, 179);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, side_s + tg, side_t + tg, 179, 179, 179);
          }
            
          if (world.block_at (bpos + v3i (x, y, z + 1)).empty ())
          {
            // top
            *vertices++ = Vertex (x + 1, y + 1, z + 1, top_s +  0, top_t +  0, 255, 255, 255);
            *vertices++ = Vertex (x + 0, y + 1, z + 1, top_s +  0, top_t + tg, 255, 255, 255);
            *vertices++ = Vertex (x + 1, y + 0, z + 1, top_s + tg, top_t +  0, 255, 255, 255);
            *vertices++ = Vertex (x + 0, y + 0, z + 1, top_s + tg, top_t + tg, 255, 255, 255);
          }
            
          if (world.block_at (bpos + v3i (x, y, z - 1)).empty ())
          {
            // bottom
            *vertices++ = Vertex (x + 0, y + 1, z + 0, bot_s +  0, bot_t +  0, 128, 128, 128);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, bot_s +  0, bot_t + tg, 128, 128, 128);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, bot_s + tg, bot_t +  0, 128, 128, 128);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, bot_s + tg, bot_t + tg, 128, 128, 128);
          }
        } // z
      } // y
    } // x

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

  Co::EntityClass <BlockWorld> ent_class ("BlockWorld");
  Co::EntityClassBase& block_world_class = ent_class;

} // namespace SH
