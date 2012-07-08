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
    static const float t;

    v2f top, sides, bottom;

    BlockTCoords () { }

    BlockTCoords (float ts, float tt, float ss, float st, float bs, float bt) :
      top    (t * v2f (ts, tt)),
      sides  (t * v2f (ss, st)),
      bottom (t * v2f (bs, bt))
    { }
    
  };

  const float BlockTCoords::t = 1.0f / 16.0f;

  class BlockTCoordsSet
  {
    BlockTCoords tcoords [256];

  public:
    BlockTCoordsSet ()
    {
      tcoords [blocktype_air  ] = BlockTCoords (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
      tcoords [blocktype_soil ] = BlockTCoords (2.0f, 0.0f, 2.0f, 0.0f, 2.0f, 0.0f);
      tcoords [blocktype_grass] = BlockTCoords (0.0f, 0.0f, 3.0f, 0.0f, 2.0f, 0.0f);
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
    float x, y, z;
    float s, t;
    float r, g, b;

    Vertex () { }

    Vertex (int x, int y, int z, float s, float t, float r, float g, float b) :
      x (float (x)), y (float (y)), z (float (z)),
      s (s), t (t),
      r (r), g (g), b (b)
    { }
      
  };

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
      max_index_bytes  = max_indices  * 2
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
      vertex_buffer = rc.create_stream (queue, 2 * max_vertex_bytes),
      index_buffer  = rc.create_stream (queue, 2 * max_index_bytes);
      
      static const Co::GeomAttrib attribs [3] = {
        { Co::attrib_position, Co::attrib_f32, 32,  0 },
        { Co::attrib_tcoords,  Co::attrib_f32, 32, 12 },
        { Co::attrib_colour,   Co::attrib_f32, 32, 20 },
      };

      compilation = rc.create_compilation (queue, attribs, vertex_buffer, index_buffer, Co::index_u16);
    }

    void regen_mesh (BlockWorld& world);

    void slice (uint limit)
    {
      for (int z = dim - 1; z != limit - 1; z--)
      {
        for (int y = 0; y != dim; y++)
        {
          for (int x = 0; x != dim; x++)
            blocks [x][y][z].type = blocktype_air;
        }
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
      world_dim    = 8,
      world_chunks = world_dim * world_dim * world_dim
    };

    // Parameters
    u64 seed;

    // State
    Chunk::Ptr stage [world_dim][world_dim][world_dim];
    uint       slice;
    float      last_slice;

    static Co::Texture::Ptr texture, sky_tex;

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

    void do_slice ()
    {
      for (int z = 1; z != 5; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
            chunk_at (v3i (x, y, z)) -> slice (slice);
        }
      }

      slice--;
    }

    virtual void tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta)
    {
      /*if (time > last_slice + 5.0f && slice > 8)
      {
        do_slice ();
        last_slice += 5.0f;
      }*/

      //Co::Profiler tick ("tick", *log_ptr);

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
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

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
          {
            auto& chunk = chunk_at (v3i (x, y, z));

            if (!chunk -> loaded)
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

      slice = 15;
      last_slice = 5.0f;

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
    for (int z = 0; z != dim; z++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int x = 0; x != dim; x++)
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
  void Chunk::regen_mesh (BlockWorld& world)
  {
    //log () << "- Regenning chunk (" << cpos.x << ", " << cpos.y << ", " << cpos.z << ")\n";

    const float tg = BlockTCoords::t;

    auto vertices = (Vertex*) vertex_buffer -> begin (max_vertex_bytes);
    if (!vertices)
      return;

    auto indices = (u16*) index_buffer -> begin (max_index_bytes);
    if (!indices)
    {
      vertex_buffer -> commit (0);
      return;
    }

    uint face_count = 0;

    for (int z = 0; z != dim; z++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int x = 0; x != dim; x++)
        {
          const auto block = at (v3i (x, y, z));

          if (block.empty ())
            continue;

          // 0, 0, 0, is the back right bottom

          auto tcoords = block_tcoords [block.type];
          float side_s = tcoords.sides.x,
                side_t = tcoords.sides.y,
                top_s  = tcoords.top.x,
                top_t  = tcoords.top.y,
                bot_s  = tcoords.bottom.x,
                bot_t  = tcoords.bottom.y;

          if (world.block_at (bpos + v3i (x + 1, y, z)).empty ())
          {
            // front
            *vertices++ = Vertex (x + 1, y + 0, z + 1, side_s +  0, side_t +  0, 0.80f, 0.80f, 0.80f);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, side_s +  0, side_t + tg, 0.80f, 0.80f, 0.80f);
            *vertices++ = Vertex (x + 1, y + 1, z + 1, side_s + tg, side_t +  0, 0.80f, 0.80f, 0.80f);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, side_s + tg, side_t + tg, 0.80f, 0.80f, 0.80f);
            face_count++;
          }

          if (world.block_at (bpos + v3i (x - 1, y, z)).empty ())
          {
            // back
            *vertices++ = Vertex (x + 0, y + 1, z + 1, side_s +  0, side_t +  0, 0.60f, 0.60f, 0.60f);
            *vertices++ = Vertex (x + 0, y + 1, z + 0, side_s +  0, side_t + tg, 0.60f, 0.60f, 0.60f);
            *vertices++ = Vertex (x + 0, y + 0, z + 1, side_s + tg, side_t +  0, 0.60f, 0.60f, 0.60f);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, side_s + tg, side_t + tg, 0.60f, 0.60f, 0.60f);
            face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y + 1, z)).empty ())
          {
            // left
            *vertices++ = Vertex (x + 1, y + 1, z + 1, side_s +  0, side_t +  0, 0.70f, 0.70f, 0.70f);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, side_s +  0, side_t + tg, 0.70f, 0.70f, 0.70f);
            *vertices++ = Vertex (x + 0, y + 1, z + 1, side_s + tg, side_t +  0, 0.70f, 0.70f, 0.70f);
            *vertices++ = Vertex (x + 0, y + 1, z + 0, side_s + tg, side_t + tg, 0.70f, 0.70f, 0.70f);
            face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y - 1, z)).empty ())
          {
            // right
            *vertices++ = Vertex (x + 0, y + 0, z + 1, side_s +  0, side_t +  0, 0.70f, 0.70f, 0.70f);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, side_s +  0, side_t + tg, 0.70f, 0.70f, 0.70f);
            *vertices++ = Vertex (x + 1, y + 0, z + 1, side_s + tg, side_t +  0, 0.70f, 0.70f, 0.70f);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, side_s + tg, side_t + tg, 0.70f, 0.70f, 0.70f);
            face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y, z + 1)).empty ())
          {
            // top
            *vertices++ = Vertex (x + 1, y + 1, z + 1, top_s +  0, top_t +  0, 1.0f, 1.0f, 1.0f);
            *vertices++ = Vertex (x + 0, y + 1, z + 1, top_s +  0, top_t + tg, 1.0f, 1.0f, 1.0f);
            *vertices++ = Vertex (x + 1, y + 0, z + 1, top_s + tg, top_t +  0, 1.0f, 1.0f, 1.0f);
            *vertices++ = Vertex (x + 0, y + 0, z + 1, top_s + tg, top_t + tg, 1.0f, 1.0f, 1.0f);
            face_count++;
          }
            
          if (world.block_at (bpos + v3i (x, y, z - 1)).empty ())
          {
            // bottom
            *vertices++ = Vertex (x + 0, y + 1, z + 0, bot_s +  0, bot_t +  0, 0.5f, 0.5f, 0.5f);
            *vertices++ = Vertex (x + 1, y + 1, z + 0, bot_s +  0, bot_t + tg, 0.5f, 0.5f, 0.5f);
            *vertices++ = Vertex (x + 0, y + 0, z + 0, bot_s + tg, bot_t +  0, 0.5f, 0.5f, 0.5f);
            *vertices++ = Vertex (x + 1, y + 0, z + 0, bot_s + tg, bot_t + tg, 0.5f, 0.5f, 0.5f);
            face_count++;
          }
        } // x
      } // y
    } // z

    uint vertex_count = 0;
    while (face_count--)
    {
      *indices++ = vertex_count + 0;
      *indices++ = vertex_count + 1;
      *indices++ = vertex_count + 2;
      *indices++ = vertex_count + 1;
      *indices++ = vertex_count + 3;
      *indices++ = vertex_count + 2;
      vertex_count += 4;
    }

    if (vertex_count != 0)
    {
      index_count = (vertex_count / 2) * 3;

      bool ok = vertex_buffer -> commit (vertex_count * sizeof (Vertex));
      if (!ok)
        return;

      ok = index_buffer  -> commit (index_count  * 2);
      if (!ok)
        return;
    }
    else
    {
      index_count = 0;
    }

    dirty = false;
  }

  Co::EntityClass <BlockWorld> ent_class ("BlockWorld");
  Co::EntityClassBase& block_world_class = ent_class;

} // namespace SH
