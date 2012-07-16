//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "World.hpp"

// Uses
#include "Common.hpp"

namespace SH
{
  Co::Spatial view_cur,
              view_next;

  static const v3i stage_corner (World::stage_radius, World::stage_radius, World::stage_radius);

  void World::tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta)
  {
    v3i view_next_cpos = floor (view_next.position / float (Chunk::dim));
    v3i stage_mins = view_next_cpos - stage_corner;

    if (view_next_cpos != view_cur_cpos)
    {
      for (int x = 0; x != stage_dim; x++)
      {
        for (int y = 0; y != stage_dim; y++)
        {
          for (int z = 0; z != stage_dim; z++)
            stage [x][y][z] = load_chunk (v3i (x, y, z) + stage_mins);
        }
      }
    }

    //Co::Profiler tick ("tick", *log_ptr);

    for (int x = 0; x != stage_dim; x++)
    {
      for (int y = 0; y != stage_dim; y++)
      {
        for (int z = 0; z != stage_dim; z++)
        {
          auto& chunk = stage [x][y][z];

          if (!chunk)
            chunk = load_chunk (v3i (x, y, z) + stage_mins);

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

  Chunk::Ptr World::load_chunk (v3i cpos)
  {
    auto iter = cache.find (cpos);

    if (iter == cache.end ())
    {
      auto chunk = std::make_shared <Chunk> (cpos);
      cache.insert (std::make_pair (cpos, chunk));
      return std::move (chunk);
    }
    else
    {
      return iter -> second;
    }
  }

  void World::render (Co::Frame& frame, float alpha)
  {
    // TODO:
    //   Frustum cull chunks
    //   Sort chunks front to back
    //   Occlusion culling (oh boy)

    //auto view = lerp (view_cur, view_next, alpha);

    Co::Material material = nil;
    material.diffuse_tex = texture -> get ();

    bool loaded [stage_dim][stage_dim][stage_dim];

    for (int x = 0; x != stage_dim; x++)
    {
      for (int y = 0; y != stage_dim; y++)
      {
        for (int z = 0; z != stage_dim; z++)
        {
          auto& chunk = stage [x][y][z];
          loaded [x][y][z] = chunk && chunk -> loaded;
        }
      }
    }

    for (int x = 1; x != stage_dim - 1; x++)
    {
      for (int y = 1; y != stage_dim - 1; y++)
      {
        for (int z = 1; z != stage_dim - 1; z++)
        {
          auto& chunk = stage [x][y][z];

          bool ok =
            loaded [x][y][z]
            && loaded [x - 1][y][z] && loaded [x + 1][y][z]
            && loaded [x][y - 1][z] && loaded [x][y + 1][z]
            && loaded [x][y][z - 1] && loaded [x][y][z + 1];

          if (!ok)
            continue;

          if (chunk -> dirty)
            chunk -> regen_mesh (*this, v3i (x, y, z));
            
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

  World::World (Co::WorkQueue& queue, const Co::PropMap* props)
  {
    view_cur_cpos = v3i (0, 0, 0);

    seed = 0xfeedbeef;

    texture = texture_factory -> create (queue, "blocks.cotexture", false, false, false);
    sky_tex = texture_factory -> create (queue, "title.cotexture",  false, true, true);
  }

  const Chunk::Ptr& World::chunk_at (v3i cv)
  {
    if (
      cv.x >= stage_dim || cv.x < 0 ||
      cv.y >= stage_dim || cv.y < 0 ||
      cv.z >= stage_dim || cv.z < 0)
    {
      static const Chunk::Ptr null = nullptr;
      return null;
    }

    return stage [cv.x][cv.y][cv.z];
  }

  Block World::block_at (v3i bv)
  {
    v3i cv (bv.x >> 4, bv.y >> 4, bv.z >> 4) ;
    bv = v3i (bv.x & 0xf, bv.y & 0xf, bv.z & 0xf);

    if (cv.x >= stage_dim || cv.y >= stage_dim || cv.z >= stage_dim || cv.x < 0 || cv.y < 0 || cv.z < 0)
      return Block (blocktype_void);
    else
      return chunk_at (cv) -> at (bv);
  }

  Co::Texture::Ptr World::sky_tex,
                   World::texture;

  Co::Entity::Ptr create_world (Co::WorkQueue& queue, const Co::PropMap* props)
  {
    return queue.gc_attach (new World (queue, props));
  }

} // namespace SH
