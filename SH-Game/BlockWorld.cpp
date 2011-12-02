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
#include <Co/Frame.hpp>

#include <Rk/Lerp.hpp>

extern Co::IxTextureFactory* texture_factory;

namespace
{
  enum BlockType :
    u8
  {
    blocktype_air   = 0x00,
    blocktype_solid = 0x01
  };
  
  class Block
  {
  public:
    BlockType type;

    bool empty () const
    {
      return type == blocktype_air;
    }

  };

  struct Vertex
  {
    float x, y, z;
    float s, t;

    Vertex () { }

    Vertex (int x, int y, int z, float s, float t) :
      x (float (x)), y (float (y)), z (float (z)), s (s), t (t)
    { }
      
  };

  class Noise
  {
    u8          permute   [256];
    Co::Vector3 gradients [256];

    static float ease_1 (float t)
    {
      t = 1.0f - t;
      float t2 = t * t;
      return (3.0f * t2) - (2.0f * t2 * t);
    }

    /*static float ease_3 (const Co::Vector3& t)
    {
      return ease_1 (std::abs (t.x)) * ease_1 (std::abs (t.y)) * ease_1 (std::abs (t.z));
    }*/

    Co::Vector3 grad (int x, int y, int z) const
    {
      return gradients [
        (x + permute [(y + permute [z & 0xff]) & 0xff]) & 0xff
      ];
    }

  public:
    void generate ()
    {
      std::srand (2112);

      for (uint i = 0; i != 256; i++)
        permute [i] = i;
      
      for (uint i = 0; i != 256; i++)
      {
        u8 other = std::rand () & 0xff;
        std::swap (permute [i], permute [other]);
      }

      // Non-uniform, but okay for now.
      for (uint i = 0; i != 256; i++)
      {
        auto vec = Co::Vector3 (
          (float (std::rand ()) / float (RAND_MAX / 2)) - 1.0f,
          (float (std::rand ()) / float (RAND_MAX / 2)) - 1.0f,
          (float (std::rand ()) / float (RAND_MAX / 2)) - 1.0f
        ).unit ();
        gradients [i] = vec;
        /*if (vec.magnitude () > 1.0f)
          return;*/
      }
    }

    void get_gradients (int x, int y, int z, Co::Vector3 grads [8]) const
    {
      for (uint i = 0; i != 8; i++)
      {
        grads [i] = grad (
          x + ((i >> 0) & 1),
          y + ((i >> 1) & 1),
          z + ((i >> 2) & 1)
        );
        /*if (grads [i].magnitude () > 1.0f)
          return;*/
      }
    }

    static float get (Co::Vector3 pos, const Co::Vector3 grads [8])
    {
      float contribs [8];
      for (uint i = 0; i != 8; i++)
      {
        auto delta = pos - Co::Vector3 (float ((i >> 0) & 1), float ((i >> 1) & 1), float ((i >> 2) & 1));
        contribs [i] = dot (grads [i], delta);
      }

      float alpha_x = ease_1 (std::abs (pos.x));
      float xiy0z0 = Rk::lerp (contribs [0], contribs [1], alpha_x);
      float xiy1z0 = Rk::lerp (contribs [2], contribs [3], alpha_x);
      float xiy0z1 = Rk::lerp (contribs [4], contribs [5], alpha_x);
      float xiy1z1 = Rk::lerp (contribs [6], contribs [7], alpha_x);

      float alpha_y = ease_1 (std::abs (pos.y));
      float xiyiz0 = Rk::lerp (xiy0z0, xiy1z0, alpha_y);
      float xiyiz1 = Rk::lerp (xiy0z1, xiy1z1, alpha_y);

      float alpha_z = ease_1 (std::abs (pos.z));
      float xiyizi = Rk::lerp (xiyiz0, xiyiz1, alpha_z);

      /*if (xiyizi > 1.0f)
        return xiyizi;*/

      return xiyizi;
    }

  };

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
    uptr vertex_count,
         index_count;

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

    void regen_mesh (Vertex* vertices, u16* indices, uint& triangle_count, float tg)
    {
      Vertex* v = vertices;
      u16*    i = indices;

      vertex_count = 0;

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

            // Correct; these calculations must be integral
            float s = float (block.type % 8) * tg,
                  t = float (block.type / 8) * tg;

            uint faces_added = 0;

            if ((x == dim - 1) || at (x + 1, y, z).empty ())
            {
              // front
              *v++ = Vertex (x + 1, y + 0, z + 1, s +  0, t + tg);
              *v++ = Vertex (x + 1, y + 0, z + 0, s +  0, t +  0);
              *v++ = Vertex (x + 1, y + 1, z + 1, s + tg, t + tg);
              *v++ = Vertex (x + 1, y + 1, z + 0, s + tg, t +  0);
              faces_added++;
            }

            if ((x == 0) || at (x - 1, y, z).empty ())
            {
              // back
              *v++ = Vertex (x + 0, y + 1, z + 1, s +  0, t + tg);
              *v++ = Vertex (x + 0, y + 1, z + 0, s +  0, t +  0);
              *v++ = Vertex (x + 0, y + 0, z + 1, s + tg, t + tg);
              *v++ = Vertex (x + 0, y + 0, z + 0, s + tg, t +  0);
              faces_added++;
            }
            
            if ((y == dim - 1) || at (x, y + 1, z).empty ())
            {
              // left
              *v++ = Vertex (x + 1, y + 1, z + 1, s +  0, t + tg);
              *v++ = Vertex (x + 1, y + 1, z + 0, s +  0, t +  0);
              *v++ = Vertex (x + 0, y + 1, z + 1, s + tg, t + tg);
              *v++ = Vertex (x + 0, y + 1, z + 0, s + tg, t +  0);
              faces_added++;
            }
            
            if ((y == 0) || at (x, y - 1, z).empty ())
            {
              // right
              *v++ = Vertex (x + 0, y + 0, z + 1, s +  0, t + tg);
              *v++ = Vertex (x + 0, y + 0, z + 0, s +  0, t +  0);
              *v++ = Vertex (x + 1, y + 0, z + 1, s + tg, t + tg);
              *v++ = Vertex (x + 1, y + 0, z + 0, s + tg, t +  0);
              faces_added++;
            }
            
            if ((z == dim - 1) || at (x, y, z + 1).empty ())
            {
              // top
              *v++ = Vertex (x + 1, y + 1, z + 1, s +  0, t + tg);
              *v++ = Vertex (x + 0, y + 1, z + 1, s +  0, t +  0);
              *v++ = Vertex (x + 1, y + 0, z + 1, s + tg, t + tg);
              *v++ = Vertex (x + 0, y + 0, z + 1, s + tg, t +  0);
              faces_added++;
            }
            
            if ((z == 0) || at (x, y, z - 1).empty ())
            {
              // bottom
              *v++ = Vertex (x + 0, y + 1, z + 0, s +  0, t + tg);
              *v++ = Vertex (x + 1, y + 1, z + 0, s +  0, t +  0);
              *v++ = Vertex (x + 0, y + 0, z + 0, s + tg, t + tg);
              *v++ = Vertex (x + 1, y + 0, z + 0, s + tg, t +  0);
              faces_added++;
            }

            while (faces_added--)
            {
              *i++ = vertex_count + 0;
              *i++ = vertex_count + 1;
              *i++ = vertex_count + 2;
              *i++ = vertex_count + 1;
              *i++ = vertex_count + 3;
              *i++ = vertex_count + 2;
              vertex_count += 4;
              triangle_count += 2;
            }
          } // x
        } // y
      } // z

      index_count = i - indices;
    }

    float generate (int cx, int cy, int cz, const Noise& noise)
    {
      Co::Vector3 grads [8];
      noise.get_gradients (cx, cy, cz, grads);
      float max_value = 0.0f;

      for (int z = 0; z != dim; z++)
      {
        for (int y = 0; y != dim; y++)
        {
          for (int x = 0; x != dim; x++)
          {
            Co::Vector3 block_pos (float (x) / dim + 0.5f, float (y) / dim + 0.5f, float (z) / dim + 0.5f);
            float value = noise.get (block_pos, grads);
            max_value = std::max (value, max_value);
            blocks [x][y][z].type = (value > 0.2f) ? blocktype_solid : blocktype_air;
          }
        }
      }

      return max_value;
    }

  }; // class Chunk

  class BlockWorld :
    public Co::IxEntity,
    public Co::IxResource
  {
    enum
    {
      world_dim    = 8,
      world_chunks = world_dim * world_dim * world_dim
    };

    Chunk                      chunks [world_dim][world_dim][world_dim];
    Co::IxGeomBuffer::Ptr      elements,
                               indices;
    Co::IxGeomCompilation::Ptr compilation;
    Co::IxTexture::Ptr         texture;
    Noise                      noise;

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
      noise.generate ();

      float max_value = 0.0f;

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
          {
            float value = chunks [x][y][z].generate (x, y, z, noise);
            max_value = std::max (value, max_value);
          }
        }
      }

      enum
      {
        max_vertices    = Chunk::max_vertices * world_chunks,
        vertex_buf_size = max_vertices * sizeof (Vertex),
        max_indices     = Chunk::max_indices * world_chunks,
        index_buf_size  = max_indices * 2
      };

      auto vertex_buf = new Vertex [max_vertices];
      auto index_buf  = new u16    [max_indices];
      uptr vertex_offset = 0,
           index_offset  = 0;

      uint triangle_count = 0;

      for (int z = 0; z != world_dim; z++)
      {
        for (int y = 0; y != world_dim; y++)
        {
          for (int x = 0; x != world_dim; x++)
          {
            chunks [x][y][z].regen_mesh (
              vertex_buf + vertex_offset,
              index_buf + index_offset,
              triangle_count,
              1.0f / 16.0f
            );

            vertex_offset += Chunk::max_vertices;
            index_offset  += Chunk::max_indices;
          }
        }
      }

      elements = rc.create_buffer (vertex_buf_size, vertex_buf);
      indices  = rc.create_buffer (index_buf_size,  index_buf );

      Co::GeomAttrib attribs [2] = {
        { Co::attrib_position, Co::attrib_f32, sizeof (Vertex),  0 },
        { Co::attrib_tcoords,  Co::attrib_f32, sizeof (Vertex), 12 }
      };

      compilation = rc.create_compilation (attribs, 2, elements, indices, Co::index_u16);

      delete [] vertex_buf;
      delete [] index_buf;

      ready = true;
    }

    virtual void tick (float time, float prev_time, Co::Frame& frame)
    {
      if (!ready)
        return;

      uint chunk_index = 0;

      for (uint z = 0; z != world_dim; z++)
      {
        float fz = float (z) * float (Chunk::dim);

        for (uint y = 0; y != world_dim; y++)
        {
          float fy = float (y) * float (Chunk::dim);

          for (uint x = 0; x != world_dim; x++)
          {
            float fx = float (x) * float (Chunk::dim);

            frame.begin_point_geom (
              compilation,
              Co::Spatial (Co::Vector3 (fx, fy, fz), Co::Quaternion ()),
              Co::Spatial (Co::Vector3 (fx, fy, fz), Co::Quaternion ())
            );

            Co::Mesh mesh = {
              Co::prim_triangles,
              0,
              chunk_index * Chunk::max_vertices,
              chunk_index * Chunk::max_indices,
              chunks [x][y][z].index_count
            };

            frame.add_meshes (&mesh, 1);

            static const Co::Material material = {
              Co::Vector4 (0, 0, 0, 0),
              Co::Vector4 (0, 0, 0, 0),
              Co::Vector4 (0, 0, 0, 0),
              Co::Vector4 (0, 0, 0, 0),
              0.0f,
              texture -> get (),
              0, 0, 0, 0
            };

            frame.add_materials (&material, 1);

            frame.end_point_geom ();

            chunk_index++;
          }
        }
      }
    }

  public:
    BlockWorld (Co::IxLoadContext& load_context, Co::IxPropMap* props)
    {
      texture = texture_factory -> create (load_context, "Blocks.cotexture", false, false);
      load_context.load (this);
    }

  };

  Co::EntityClass <BlockWorld> ent_class ("BlockWorld");

}

Co::IxEntityClass* block_world_class = &ent_class;
