//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "World.hpp"

// Uses
#include <Co/EntityClass.hpp>

#include "Common.hpp"

namespace SH
{
  void World::do_slice ()
  {
    for (int x = 0; x != world_dim; x++)
    {
      for (int y = 0; y != world_dim; y++)
        stage [x][y][slice >> 4] -> slice (slice);
    }

    slice--;
  }

  void World::tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta)
  {
    /*if (time >= last_slice + 1.0f && slice > 45)
    {
      do_slice ();
      last_slice += 1.0f;
    }*/

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

  void World::render (Co::Frame& frame, float alpha)
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

  World::World (Co::WorkQueue& queue)
  {
    seed = 0xfeedbeef;

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

  const Chunk::Ptr& World::chunk_at (v3i cv)
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

  Block World::block_at (v3i bv)
  {
    v3i cv (bv.x >> 4, bv.y >> 4, bv.z >> 4) ;
    bv = v3i (bv.x & 0xf, bv.y & 0xf, bv.z & 0xf);

    if (cv.x >= world_dim || cv.y >= world_dim || cv.z >= world_dim || cv.x < 0 || cv.y < 0 || cv.z < 0)
      return Block (blocktype_void);
    else
      return chunk_at (cv) -> at (bv);
  }

  Co::Texture::Ptr World::sky_tex,
                   World::texture;

  Co::EntityClass <World> ent_class ("World");
  Co::EntityClassBase& world_class = ent_class;

} // namespace SH
