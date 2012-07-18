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
      for (v3i c (0, 0, 0); c.x != stage_dim; advance_cubic_zyx (c, stage_dim))
        stage [c.x][c.y][c.z] = load_chunk (c + stage_mins);
    }

    //Co::Profiler tick ("tick", *log_ptr);

    for (v3i c (0, 0, 0); c.x != stage_dim; advance_cubic_zyx (c, stage_dim))
    {
      auto& chunk = stage [c.x][c.y][c.z];

      if (!chunk)
        chunk = load_chunk (c + stage_mins);

      if (!chunk -> loaded)
      {
        if (!chunk -> loading)
          chunk -> generate (chunk, queue, seed);
        continue;
      }

      // ...
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
    // Frustum culling
    auto view = lerp (view_cur, view_next, alpha);

    float recip_aspect = float (frame.height) / float (frame.width);
    float x_hfov = 75.0f * (3.141592f / 180.0f) * 0.5f;
    float y_hfov = x_hfov * recip_aspect;

    v3f fwd  = view.orientation.forward (),
        left = view.orientation.left    (),
        up   = view.orientation.up      ();

    v3f v_fwd_contrib  = std::sin (y_hfov) * fwd,
        h_fwd_contrib  = std::sin (x_hfov) * fwd,
        v_up_contrib   = std::cos (y_hfov) * up,
        h_left_contrib = std::cos (x_hfov) * left;

    v3f planes [4] = {
      v_fwd_contrib - v_up_contrib,   // top
      v_fwd_contrib + v_up_contrib,   // bottom
      h_fwd_contrib - h_left_contrib, // left
      h_fwd_contrib + h_left_contrib  // right
    };

    // TODO:
    //   Sort chunks front to back
    //   Occlusion culling (oh boy)


    Co::Material material = nil;
    material.diffuse_tex = texture -> get ();

    bool loaded [stage_dim][stage_dim][stage_dim];

    for (v3i c (0, 0, 0); c.x != stage_dim; advance_cubic_zyx (c, stage_dim))
    {
      auto& chunk = stage [c.x][c.y][c.z];
      loaded [c.x][c.y][c.z] = chunk && chunk -> loaded;
    }

    for (v3i c (0, 0, 0); c.x != stage_dim - 2; advance_cubic_zyx (c, stage_dim - 2))
    {
      auto cp = c + v3i (1, 1, 1);

      auto& chunk = stage [cp.x][cp.y][cp.z];

      bool draw =
        loaded [cp.x][cp.y][cp.z]
        && loaded [cp.x - 1][cp.y][cp.z] && loaded [cp.x + 1][cp.y][cp.z]
        && loaded [cp.x][cp.y - 1][cp.z] && loaded [cp.x][cp.y + 1][cp.z]
        && loaded [cp.x][cp.y][cp.z - 1] && loaded [cp.x][cp.y][cp.z + 1];

      if (!draw)
        continue;

      // Frustum cull chunk
      for (uint i = 0; i != 4; i++)
      {
        v3f bp = chunk -> bpos;
        v3f centre = bp + (v3i (Chunk::dim, Chunk::dim, Chunk::dim) / 2) - view.position;
        static const float radius = float (Chunk::dim / 2) * 1.73205f; // 8rt3
        if (dot (centre, planes [i]) < -radius)
        {
          draw = false;
          break;
        }
      }

      if (!draw)
        continue;

      if (chunk -> dirty)
        chunk -> regen_mesh (*this, cp);
            
      if (chunk -> index_count > 0)
        chunk -> draw (frame, material);
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

  /*Co::Texture::Ptr World::sky_tex,
                   World::texture;*/

  Co::Entity::Ptr create_world (Co::WorkQueue& queue, const Co::PropMap* props)
  {
    return queue.gc_attach (new World (queue, props));
  }

} // namespace SH
