//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_NOISE
#define SH_H_NOISE

// Uses
#include <Rk/Vector3.hpp>
#include <Rk/Lerp.hpp>

#include <algorithm>

namespace SH
{
  class RandomGen
  {
    u64 state;

  public:
    RandomGen () :
      state (0)
    { }
    
    RandomGen (u32 seed) :
      state (seed)
    { }
    
    void set_seed (u32 seed)
    {
      state = seed;
    }

    u32 get ()
    {
      state = state * 0x5851f42d4c957f2dull + 0x14057b7ef767814full;
      return (state >> 32) & 0xffffffff;
    }

  };

  inline float ease (float t)
  {
    const auto t2 = t * t;
    return /*t * t * t * (t * (t * 6 - 15) + 10);*/3.0f * t2 - 2.0f * t2 * t;//6.0f * std::pow (t, 5) - 15.0f * std::pow (t, 4) + 10.0f * std::pow (t, 3);
  }

  inline v2f ease (v2f t)
  {
    return v2f (ease (t.x), ease (t.y));
  }

  inline v3f ease (v3f t)
  {
    return v3f (ease (t.x), ease (t.y), ease (t.z));
  }

  inline float bilerp (float x0y0, float x1y0, float x0y1, float x1y1, v2f t)
  {
    return Rk::lerp (
      Rk::lerp (x0y0, x1y0, t.x),
      Rk::lerp (x0y1, x1y1, t.x),
      t.y
    );
  }

  inline float trilerp (
    float x0y0z0, float x1y0z0, float x0y1z0, float x1y1z0,
    float x0y0z1, float x1y0z1, float x0y1z1, float x1y1z1,
    v3f t)
  {
    return Rk::lerp (
      bilerp (x0y0z0, x1y0z0, x0y1z0, x1y1z0, t.xy ()),
      bilerp (x0y0z1, x1y0z1, x0y1z1, x1y1z1, t.xy ()),
      t.z
    );
  }

  static const i32 x_noise    = 73856093,
                   y_noise    = 19349663,
                   z_noise    = 83492791,
                   seed_noise = 37667659;

  extern v2f noise_grads_2d [256];

  inline float noise_grad (v2f p, v2i cell, u32 seed)
  {
    u32 index = hash (cell) ^ (seed_noise * seed);
    return dot (p - cell, noise_grads_2d [index & 0xff]);
  }

  extern v3f noise_grads_3d [32];

  inline float noise_grad (v3f p, v3i cell, u32 seed)
  {
    u32 index = hash (cell) ^ (seed_noise * seed);
    //return dot (p - cell, noise_grads_3d [index & 0x1f]);

    index &= 0xf;
    p -= cell;
    float u = (index < 8) ? p.x : p.y;
    float v = (index < 4) ? p.y : (index == 12 || index == 14) ? p.x : p.z;
    return ((index & 1) ? -u : u) + ((index & 2) ? -v : v);
  }

  extern float perlin_2d_min,
               perlin_2d_max;

  static float noise_perlin (v2f p, u32 seed)
  {
    v2i cell = floor (p);
    
    float result = bilerp (
      noise_grad (p, cell + v2i (0, 0), seed),
      noise_grad (p, cell + v2i (1, 0), seed),
      noise_grad (p, cell + v2i (0, 1), seed),
      noise_grad (p, cell + v2i (1, 1), seed),
      ease (p - cell)
    );

    perlin_2d_min = std::min (perlin_2d_min, result);
    perlin_2d_max = std::max (perlin_2d_max, result);

    return result;
  }

  static float noise_perlin (v3f p, u32 seed)
  {
    v3i cell = floor (p);
    
    return trilerp (
      noise_grad (p, cell + v3i (0, 0, 0), seed),
      noise_grad (p, cell + v3i (1, 0, 0), seed),
      noise_grad (p, cell + v3i (0, 1, 0), seed),
      noise_grad (p, cell + v3i (1, 1, 0), seed),
      noise_grad (p, cell + v3i (0, 0, 1), seed),
      noise_grad (p, cell + v3i (1, 0, 1), seed),
      noise_grad (p, cell + v3i (0, 1, 1), seed),
      noise_grad (p, cell + v3i (1, 1, 1), seed),
      ease (p - cell)
    );
  }
  
  template <typename Vec>
  static float noise_perlin_harmonic (Vec p, u32 seed, float base, u32 octaves, float persistence)
  {
    float result    = 0.0f,
          scale     = 1.0f,
          frequency = base;

    for (uint octave = 0; octave != octaves; octave++)
    {
      result += scale * noise_perlin (p * frequency, seed + octave);
      frequency *= 2.0f;
      scale     *= persistence;
    }

    return result;
  }
  
}

#endif
