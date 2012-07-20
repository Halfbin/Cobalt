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
  void Chunk::generate (Ptr self, Co::WorkQueue& queue, u32 seed)
  {
    log () << "- Generating chunk at (" << cpos.x << ", " << cpos.y << ", " << cpos.z << ")\n";

    auto on_complete = [self]
    {
      self -> loaded = true;
      self -> dirty  = true;
    };

    auto on_load = [self, seed, on_complete] (Co::WorkQueue& queue, Co::RenderContext&, Co::Filesystem&)
    {
      self -> generate_impl (seed);
      queue.queue_completion (on_complete);
    };

    queue.queue_load (on_load);
  }

  void Chunk::generate_impl (u32 seed)
  {
    // Land
    if (bpos.z > 85)
    {
      for (auto b = iteration (v3i (0, 0, 0), chunk_extent); b; b.advance ())
        blocks [b.x][b.y][b.z].type = blocktype_air;
    }
    else// if (bpos.z > 25)
    {
      for (int x = 0; x != chunk_dim; x++)
      {
        for (int y = 0; y != chunk_dim; y++)
        {
          v2f noise_pos = cpos.xy () + v2f (x, y) * (1.0f / chunk_dim);
          int level = int (64.0f + 22.0f * noise_perlin_harmonic (noise_pos, seed, 0.3f, 3, 0.25f));
          level -= bpos.z;

          int depth = int (8.0f + 10.0f * noise_perlin_harmonic (noise_pos, seed + 20, 0.3f, 3, 0.25f));
          depth = level - depth;

          int z = 0;

          while (z < std::min (depth, chunk_dim))
            blocks [x][y][z++].type = blocktype_stone;

          while (z < std::min (level, chunk_dim))
            blocks [x][y][z++].type = blocktype_soil;

          if (level >= 0 && level < chunk_dim)
            blocks [x][y][z++].type = blocktype_grass;

          while (z < chunk_dim)
            blocks [x][y][z++].type = blocktype_air;
        }
      }
    }/*
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
    }*/

    // Caves
    if (bpos.z < 65)
    {
      for (auto b = iteration (v3i (0, 0, 0), chunk_extent); b; b.advance ())
      {
        v3f noise_pos = cpos + v3f (b) * (1.0f / chunk_dim);
        bool cut = -0.2f > noise_perlin_harmonic (noise_pos, seed, 0.5f, 3, 0.8f);

        if (cut)
          blocks [b.x][b.y][b.z].type = blocktype_air;
      }
    }

    regen_caches ();
  }

  void Chunk::regen_caches ()
  {
    for (int x = 0; x != chunk_dim; x++)
    {
      for (int y = 0; y != chunk_dim; y++)
      {
        opacity  [x][y] = 0;
        //solidity [x][y] = 0;
        for (int z = 0; z != chunk_dim; z++)
          opacity [x][y] |= u16 (!blocks [x][y][z].empty ()) << z;
      }
    }
  }

} // namespace SH
