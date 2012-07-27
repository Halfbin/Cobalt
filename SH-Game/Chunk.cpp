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

#include <Co/Clock.hpp>

namespace SH
{
  static void log_gen_time (const Chunk& chunk)
  {
    log () << "i gen_time: " << chunk.get_gen_time () << '\n';
  }

  void Chunk::generate (Ptr self, Co::WorkQueue& queue, u32 seed)
  {
    //log () << "- Generating chunk at (" << cpos.x << ", " << cpos.y << ", " << cpos.z << ")\n";

    auto on_complete = [self]
    {
      //log_gen_time (*self);
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
    //Co::Clock prof;

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
          int level = int (64.0f + 22.0f * noise_perlin_harmonic (noise_pos, seed, 0.3f, 2, 0.25f));
          level -= bpos.z;

          int depth = int (8.0f + 10.0f * noise_perlin_harmonic (noise_pos, seed + 20, 0.3f, 2, 0.25f));
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
    /*if (bpos.z < 62)
    {
      for (auto b = iteration (v3i (0, 0, 0), chunk_extent); b; b.advance ())
      {
        v3f noise_pos = cpos + v3f (b) * (1.0f / chunk_dim);
        bool cut = -0.2f > noise_perlin_harmonic (noise_pos, seed, 0.5f, 2, 0.8f);

        if (cut)
          blocks [b.x][b.y][b.z].type = blocktype_air;
      }
    }*/

    regen_caches ();

    //gen_time = prof.time ();
  }

  void Chunk::regen_caches ()
  {
    // opacity caches
    for (int y = 0; y != chunk_dim; y++)
    {
      for (int z = 0; z != chunk_dim; z++)
      {
        x_opacity [y][z] = 0;
        //solidity [x][y] = 0;
        for (int x = 0; x != chunk_dim; x++)
          x_opacity [y][z] |= u16 (!blocks [x][y][z].empty ()) << x;
      }
    }

    for (int x = 0; x != chunk_dim; x++)
    {
      for (int z = 0; z != chunk_dim; z++)
      {
        y_opacity [x][z] = 0;
        //solidity [x][y] = 0;
        for (int y = 0; y != chunk_dim; y++)
          y_opacity [x][z] |= u16 (!blocks [x][y][z].empty ()) << y;
      }
    }

    for (int x = 0; x != chunk_dim; x++)
    {
      for (int y = 0; y != chunk_dim; y++)
      {
        z_opacity [x][y] = 0;
        //solidity [x][y] = 0;
        for (int z = 0; z != chunk_dim; z++)
          z_opacity [x][y] |= u16 (!blocks [x][y][z].empty ()) << z;
      }
    }

    // internal face count
    internal_face_count = 0;

    for (int x = 0; x != chunk_dim; x++)
    {
      for (int y = 0; y != chunk_dim; y++)
      {
        u16 bits = z_opacity [x][y];
        u16 trans = (bits ^ rotr (bits, 1)) & 0x7fff;
        internal_face_count += bit_count (trans);
      }
    }

    for (int x = 0; x != chunk_dim; x++)
    {
      for (int z = 0; z != chunk_dim; z++)
      {
        u16 bits = y_opacity [x][z];
        u16 trans = (bits ^ rotr (bits, 1)) & 0x7fff;
        internal_face_count += bit_count (trans);
      }
    }

    for (int y = 0; y != chunk_dim; y++)
    {
      for (int z = 0; z != chunk_dim; z++)
      {
        u16 bits = x_opacity [y][z];
        u16 trans = (bits ^ rotr (bits, 1)) & 0x7fff;
        internal_face_count += bit_count (trans);
      }
    }
  }

  float Chunk::get_gen_time () const
  {
    return gen_time;
  }

  uptr total_cmem = 0;
  uptr high_cmem  = 0;

  uptr Chunk::high_mem ()
  {
    return high_cmem;
  }

  Chunk::Chunk (v3i cpos) :
    loaded  (false),
    dirty   (false),
    cpos    (cpos),
    bpos    (cpos * chunk_dim)
  {
    total_cmem += sizeof (Chunk);
    if (total_cmem > high_cmem)
      high_cmem = total_cmem;
  }
  
  Chunk::~Chunk ()
  {
    total_cmem -= sizeof (Chunk);
  }

} // namespace SH
