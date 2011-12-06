//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxResource.hpp>
#include <Co/IxEntity.hpp>

// Uses
#include <Co/IxGeomCompilation.hpp>
#include <Co/IxTextureFactory.hpp>
#include <Co/IxRenderContext.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/IxGeomBuffer.hpp>
#include <Co/EntityClass.hpp>
#include <Co/IxTexture.hpp>
#include <Co/IxFrame.hpp>

#include <Rk/VirtualOutStream.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Lerp.hpp>

#include <vector>

namespace SH
{
  extern Rk::VirtualLockedOutStream log;
  extern Co::IxTextureFactory* texture_factory;

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

    Co::Vector2 top,
                sides,
                bottom;

    BlockTCoords () { }

    BlockTCoords (float ts, float tt, float ss, float st, float bs, float bt) :
      top    (t * Co::Vector2 (ts, tt)),
      sides  (t * Co::Vector2 (ss, st)),
      bottom (t * Co::Vector2 (bs, bt))
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
      tcoords [blocktype_soil ] = BlockTCoords (3.0f, 0.0f, 3.0f, 0.0f, 3.0f, 0.0f);
      tcoords [blocktype_grass] = BlockTCoords (2.0f, 0.0f, 1.0f, 0.0f, 3.0f, 0.0f);
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

  class Noise
  {
    static const uint size = 65536,
                      mask = size - 1;

    uint        permute   [size];
    Co::Vector3 gradients [size];

    static float ease_1 (float t)
    {
      //t = 1.0f - t;
      float t2 = t * t;
      return (3.0f * t2) - (2.0f * t2 * t);
    }

    Co::Vector3 grad (int x, int y, int z) const
    {
      return gradients [
        (x + permute [(y + permute [z & mask]) & mask]) & mask
      ];
    }

  public:
    void generate (u64 seed)
    {
      std::srand (seed & 0xffffffff);

      for (uint i = 0; i != size; i++)
        permute [i] = i;
      
      for (uint i = 0; i != size; i++)
      {
        uint other = std::rand () & mask;
        std::swap (permute [i], permute [other]);
      }

      // Non-uniform, but okay for now.
      for (uint i = 0; i != size; i++)
      {
        uint samples [3] = { 0, 0, 0 };
        for (uint s = 0; s != 3; s++)
          for (uint r = 0; r != 10; r++)
            samples [s] += uint (std::rand ());

        auto vec = Co::Vector3 (
          (float (samples [0]) / float (RAND_MAX * 5)) - 1.0f,
          (float (samples [1]) / float (RAND_MAX * 5)) - 1.0f,
          (float (samples [2]) / float (RAND_MAX * 5)) - 1.0f
        ).unit ();
        gradients [i] = vec;
        /*if (vec.magnitude () > 1.0f)
          return;*/
      }
    }

    void get_gradients (int x, int y, int z, Co::Vector3 grads [2][2][2]) const
    {
      for (uint k = 0; k != 2; k++)
      {
        for (uint j = 0; j != 2; j++)
        {
          for (uint i = 0; i != 2; i++)
          {
            grads [i][j][k] = grad (x + i, y + j, z + k);
          }
        }
      }
    }

    static float get (Co::Vector3 pos, const Co::Vector3 grads [2][2][2])
    {
      float contribs [2][2][2];
      for (uint k = 0; k != 2; k++)
      {
        for (uint j = 0; j != 2; j++)
        {
          for (uint i = 0; i != 2; i++)
          {
            auto delta = pos - Co::Vector3 (float (i), float (j), float (k));
            contribs [i][j][k] = dot (grads [i][j][k], delta);
          }
        }
      }

      float alpha_x = ease_1 (1.0f - pos.x);
      float xiy0z0 = Rk::lerp (contribs [0][0][0], contribs [1][0][0], alpha_x);
      float xiy1z0 = Rk::lerp (contribs [0][1][0], contribs [1][1][0], alpha_x);
      float xiy0z1 = Rk::lerp (contribs [0][0][1], contribs [1][0][1], alpha_x);
      float xiy1z1 = Rk::lerp (contribs [0][1][1], contribs [1][1][1], alpha_x);

      float alpha_y = ease_1 (1.0f - pos.y);
      float xiyiz0 = Rk::lerp (xiy0z0, xiy1z0, alpha_y);
      float xiyiz1 = Rk::lerp (xiy0z1, xiy1z1, alpha_y);

      float alpha_z = ease_1 (1.0f - pos.z);
      float xiyizi = Rk::lerp (xiyiz0, xiyiz1, alpha_z);

      /*if (xiyizi > 1.0f)
        return xiyizi;*/

      return std::abs (xiyizi);
    }

  };

  class BlockWorld;

  class Chunk
  {
  public:
    enum
    {
      dim = 16,
      max_faces    = ((dim * dim * dim + 1) / 2) * 6,
      max_vertices = max_faces * 4,
      max_indices  = max_faces * 6
    };

    Block blocks [dim][dim][dim];
    
    Co::IxGeomCompilation::Ptr compilation;
    uptr index_count;

    int cx, cy, cz,
        bx, by, bz;

    void set_pos (int x, int y, int z)
    {
      cx = x; bx = cx * dim;
      cy = y; by = cy * dim;
      cz = z; bz = cz * dim;
    }

    Block& at (int x, int y, int z)
    {
      if (x == -1) x = 0; else if (x == dim) x = dim - 1;
      if (y == -1) y = 0; else if (y == dim) y = dim - 1;
      if (z == -1) z = 0; else if (z == dim) z = dim - 1;
      return blocks [x][y][z];
    }

    const Block& at (int x, int y, int z) const
    {
      if (x == -1) x = 0; else if (x == dim) x = dim - 1;
      if (y == -1) y = 0; else if (y == dim) y = dim - 1;
      if (z == -1) z = 0; else if (z == dim) z = dim - 1;
      return blocks [x][y][z];
    }

    uint regen_mesh (const BlockWorld& world, Co::IxRenderContext& rc, float tg, std::vector <Vertex>& vertices, std::vector <u16>& indices);

    void draw (Co::IxFrame* frame, const Co::Material& mat)
    {
      if (!compilation)
        return;

      float fx = float (bx),
            fy = float (by),
            fz = float (bz);

      frame -> begin_point_geom (
        compilation.get (),
        Co::Spatial (Co::Vector3 (fx, fy, fz), nil),
        Co::Spatial (Co::Vector3 (fx, fy, fz), nil)
      );

      Co::Mesh mesh (Co::prim_triangles, 0, 0, 0, index_count);

      frame -> add_meshes    (&mesh, 1);
      frame -> add_materials (&mat,  1);

      frame -> end_point_geom ();
    }

    void generate_pass_1 (const Noise& noise);
    void generate_pass_2 (const BlockWorld& world, const Noise& noise);

  }; // class Chunk

  class BlockWorld :
    public Co::IxEntity,
    public Co::IxResource
  {
    enum
    {
      world_dim    = 12,
      world_chunks = world_dim * world_dim * world_dim
    };

    // Parameters
    u64 seed;

    // State
    Chunk              chunks [world_dim][world_dim][world_dim];
    Co::IxTexture::Ptr texture;
    Noise              noise;

    static const Block outside_block;

    const Chunk& chunk_at (int x, int y, int z) const
    {
      if (
        x >= world_dim || x < 0 ||
        y >= world_dim || y < 0 ||
        z >= world_dim || z < 0)
      {
        throw Rk::Exception ("Chunk reference outside world");
      }

      return chunks [x][y][z];
    }

    Chunk& chunk_at (int x, int y, int z)
    {
      if (
        x >= world_dim || x < 0 ||
        y >= world_dim || y < 0 ||
        z >= world_dim || z < 0)
      {
        throw Rk::Exception ("Chunk reference outside world");
      }

      return chunks [x][y][z];
    }

    //enum { chunks_size_mb = sizeof (chunks) / (1024 * 1024) };

    virtual void acquire () { }
    virtual void release () { }

    virtual void dispose ()
    {
      delete this;
    }

    virtual void destroy ()
    {
      delete this;
    }

    virtual void load (Co::IxRenderContext& rc)
    {
      noise.generate (seed);

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
          {
            auto& chunk = chunk_at (x, y, z);
            chunk.set_pos (x, y, z);
            chunk.generate_pass_1 (noise);
          }
        }
      }

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
            chunk_at (x, y, z).generate_pass_2 (*this, noise);
        }
      }

      uint triangle_count = 0;

      // Scratch space for Chunk::regen_mesh only
      std::vector <Vertex> vertices;
      std::vector <u16>    indices;

      vertices.reserve (Chunk::max_vertices);
      indices.reserve  (Chunk::max_indices);

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
            triangle_count += chunk_at (x, y, z).regen_mesh (*this, rc, 1.0f / 16.0f, vertices, indices);
        }
      }

      uint face_count = triangle_count / 2;
      uint bytes_per_face = 4 * sizeof (Vertex) + 6 * sizeof (u16);

      uint bytes     = face_count * bytes_per_face;
      uint bytes_max = world_chunks * Chunk::max_faces * bytes_per_face;

      float size_mb = float (bytes) / float (1024 * 1024);
      float fill    = float (bytes) / float (bytes_max) * 100.0f;

      log << "- SH-Game: BlockWorld::load - generated " << triangle_count << " triangles ("
          << size_mb << " MB, "
          << fill    << "% capacity) \n";

      ready = true;
    }

    virtual void tick (Co::IxFrame* frame, float time, float prev_time)
    {
      if (!ready)
        return;

      Co::Material material = nil;
      material.diffuse_tex = texture -> get ();

      for (uint z = 0; z != world_dim; z++)
      {
        for (uint y = 0; y != world_dim; y++)
        {
          for (uint x = 0; x != world_dim; x++)
            chunk_at (x, y, z).draw (frame, material);
        }
      }
    }

  public:
    BlockWorld (Co::IxLoadContext* load_context, Co::IxPropMap* props)
    {
      seed = 923;
      texture = texture_factory -> create (load_context, "Blocks.cotexture", false, false);
      load_context -> load (this);
    }

    const Block& block_at (int bx, int by, int bz) const
    {
      int cx = bx / Chunk::dim,
          cy = by / Chunk::dim, 
          cz = bz / Chunk::dim;

      bx %= Chunk::dim;
      by %= Chunk::dim;
      bz %= Chunk::dim;

      if (
        cx >= world_dim || cx < 0 ||
        cy >= world_dim || cy < 0 ||
        cz >= world_dim || cz < 0)
      {
        return outside_block;
      }
      else
      {
        return chunk_at (cx, cy, cz).at (bx, by, bz);
      }
    }

  };

  const Block BlockWorld::outside_block (blocktype_void);

  void Chunk::generate_pass_1 (const Noise& noise)
  {
    Co::Vector3 grads [2][2][2];
    noise.get_gradients (cx, cy, cz, grads);
    
    for (int z = 0; z != dim; z++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int x = 0; x != dim; x++)
        {
          Co::Vector3 block_pos ((float (x) + 0.5f) / float (dim), (float (y) + 0.5f) / float (dim), (float (z) + 0.5f) / float (dim));
          float value = noise.get (block_pos, grads);
          at (x, y, z).type = (value > 0.8f) ? blocktype_soil : blocktype_air;
        }
      }
    }
  }

  void Chunk::generate_pass_2 (const BlockWorld& world, const Noise& noise)
  {
    for (int z = 0; z != dim; z++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int x = 0; x != dim; x++)
        {
          if ((at (x, y, z).type == blocktype_soil) && (world.block_at (bx + x, by + y, bz + z + 1).empty ()))
            at (x, y, z).type = blocktype_grass;
        }
      }
    }
  }

  uint Chunk::regen_mesh (const BlockWorld& world, Co::IxRenderContext& rc, float tg, std::vector <Vertex>& vertices, std::vector <u16>& indices)
  {
    uint vertex_count = 0;

    for (int z = 0; z != dim; z++)
    {
      for (int y = 0; y != dim; y++)
      {
        for (int x = 0; x != dim; x++)
        {
          const auto& block = at (x, y, z);

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

          //static const float eps = 0.0f;//1.0f / 1024.0f;

          if (world.block_at (bx + x + 1, by + y, bz + z).empty ())
          {
            // front
            vertices.emplace_back (Vertex (x + 1, y + 0, z + 1, side_s +  0, side_t +  0, 0.75f, 0.75f, 0.75f));
            vertices.emplace_back (Vertex (x + 1, y + 0, z + 0, side_s +  0, side_t + tg, 0.75f, 0.75f, 0.75f));
            vertices.emplace_back (Vertex (x + 1, y + 1, z + 1, side_s + tg, side_t +  0, 0.75f, 0.75f, 0.75f));
            vertices.emplace_back (Vertex (x + 1, y + 1, z + 0, side_s + tg, side_t + tg, 0.75f, 0.75f, 0.75f));
          }

          if (world.block_at (bx + x - 1, by + y, bz + z).empty ())
          {
            // back
            vertices.emplace_back (Vertex (x + 0, y + 1, z + 1, side_s +  0, side_t +  0, 0.65f, 0.65f, 0.65f));
            vertices.emplace_back (Vertex (x + 0, y + 1, z + 0, side_s +  0, side_t + tg, 0.65f, 0.65f, 0.65f));
            vertices.emplace_back (Vertex (x + 0, y + 0, z + 1, side_s + tg, side_t +  0, 0.65f, 0.65f, 0.65f));
            vertices.emplace_back (Vertex (x + 0, y + 0, z + 0, side_s + tg, side_t + tg, 0.65f, 0.65f, 0.65f));
          }
            
          if (world.block_at (bx + x, by + y + 1, bz + z).empty ())
          {
            // left
            vertices.emplace_back (Vertex (x + 1, y + 1, z + 1, side_s +  0, side_t +  0, 0.75f, 0.75f, 0.75f));
            vertices.emplace_back (Vertex (x + 1, y + 1, z + 0, side_s +  0, side_t + tg, 0.75f, 0.75f, 0.75f));
            vertices.emplace_back (Vertex (x + 0, y + 1, z + 1, side_s + tg, side_t +  0, 0.75f, 0.75f, 0.75f));
            vertices.emplace_back (Vertex (x + 0, y + 1, z + 0, side_s + tg, side_t + tg, 0.75f, 0.75f, 0.75f));
          }
            
          if (world.block_at (bx + x, by + y - 1, bz + z).empty ())
          {
            // right
            vertices.emplace_back (Vertex (x + 0, y + 0, z + 1, side_s +  0, side_t +  0, 0.65f, 0.65f, 0.65f));
            vertices.emplace_back (Vertex (x + 0, y + 0, z + 0, side_s +  0, side_t + tg, 0.65f, 0.65f, 0.65f));
            vertices.emplace_back (Vertex (x + 1, y + 0, z + 1, side_s + tg, side_t +  0, 0.65f, 0.65f, 0.65f));
            vertices.emplace_back (Vertex (x + 1, y + 0, z + 0, side_s + tg, side_t + tg, 0.65f, 0.65f, 0.65f));
          }
            
          if (world.block_at (bx + x, by + y, bz + z + 1).empty ())
          {
            // top
            vertices.emplace_back (Vertex (x + 1, y + 1, z + 1, top_s +  0, top_t +  0, 1.0f, 1.0f, 1.0f));
            vertices.emplace_back (Vertex (x + 0, y + 1, z + 1, top_s +  0, top_t + tg, 1.0f, 1.0f, 1.0f));
            vertices.emplace_back (Vertex (x + 1, y + 0, z + 1, top_s + tg, top_t +  0, 1.0f, 1.0f, 1.0f));
            vertices.emplace_back (Vertex (x + 0, y + 0, z + 1, top_s + tg, top_t + tg, 1.0f, 1.0f, 1.0f));
          }
            
          if (world.block_at (bx + x, by + y, bz + z - 1).empty ())
          {
            // bottom
            vertices.emplace_back (Vertex (x + 0, y + 1, z + 0, bot_s +  0, bot_t +  0, 0.5f, 0.5f, 0.5f));
            vertices.emplace_back (Vertex (x + 1, y + 1, z + 0, bot_s +  0, bot_t + tg, 0.5f, 0.5f, 0.5f));
            vertices.emplace_back (Vertex (x + 0, y + 0, z + 0, bot_s + tg, bot_t +  0, 0.5f, 0.5f, 0.5f));
            vertices.emplace_back (Vertex (x + 1, y + 0, z + 0, bot_s + tg, bot_t + tg, 0.5f, 0.5f, 0.5f));
          }

          while (vertex_count != vertices.size ())
          {
            indices.push_back (vertex_count + 0);
            indices.push_back (vertex_count + 1);
            indices.push_back (vertex_count + 2);
            indices.push_back (vertex_count + 1);
            indices.push_back (vertex_count + 3);
            indices.push_back (vertex_count + 2);
            vertex_count += 4;
          }
        } // x
      } // y
    } // z

    if (vertex_count == 0)
      return 0;

    auto vertex_buffer = rc.create_buffer (vertices.size () * sizeof (Vertex), vertices.data ()),
         index_buffer  = rc.create_buffer (indices.size ()  * sizeof (u16),    indices.data () );
    
    static const Co::GeomAttrib attribs [3] = {
      { Co::attrib_position, Co::attrib_f32, sizeof (Vertex),  0 },
      { Co::attrib_tcoords,  Co::attrib_f32, sizeof (Vertex), 12 },
      { Co::attrib_colour,   Co::attrib_f32, sizeof (Vertex), 20 },
    };

    compilation = rc.create_compilation (attribs, vertex_buffer, index_buffer, Co::index_u16);

    uint vertices_cap = vertices.capacity ();
    vertices.clear ();
    assert (vertices.capacity () == vertices_cap);

    index_count = indices.size ();
    indices.clear ();

    uint face_count = vertex_count / 4;

    /*float fill = float (face_count) / float (max_faces) * 100.0f;
    log << "- Generated chunk at " << fill << "% capacity\n";*/

    return face_count * 2;
  }

  Co::EntityClass <BlockWorld> ent_class ("BlockWorld");
  const Co::IxEntityClass* block_world_class = &ent_class;

} // namespace SH
