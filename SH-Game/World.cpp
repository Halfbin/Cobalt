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
  Co::Spatial view_cur  (nil),
              view_next (nil);

  static const v3i stage_corner (stage_radius, stage_radius, stage_radius);
  
  void World::tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta)
  {
    v3i view_cpos = floor (view_next.position / float (chunk_dim));
    v3i view_base = view_cpos - stage_corner;

    /*if (view_cpos != view_cur_cpos)
    {*/
      stage_base = view_cpos % stage_dim;
      if (stage_base.x < 0) stage_base.x += stage_dim;
      if (stage_base.y < 0) stage_base.y += stage_dim;
      if (stage_base.z < 0) stage_base.z += stage_dim;

      for (auto c = iteration (v3i (0, 0, 0), stage_extent); c; c.advance ())
      {
        auto& chunk = stage_at (c).chunk;
        v3i new_cpos = view_base + c;

        if (!chunk || chunk -> cpos != new_cpos)
          chunk = load_chunk (queue, new_cpos);
      }

      //view_cur_cpos = view_cpos;
    //}
  }

  static bool cache_enable = false;

  Chunk::Ptr World::load_chunk (Co::WorkQueue& queue, v3i cpos)
  {
    if (cache_enable)
    {
      auto iter = cache.find (cpos);

      if (iter != cache.end ())
      {
        auto chunk = iter -> second;
        chunk -> dirty = true;
        return std::move (chunk);        
      }
    }

    auto chunk = std::make_shared <Chunk> (cpos);
    chunk -> generate (chunk, queue, seed);
    if (cache_enable)
      cache.insert (std::make_pair (cpos, chunk));
    return std::move (chunk);
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

    /*bool loaded [stage_dim][stage_dim][stage_dim];

    for (auto c = iteration (v3i (0, 0, 0), stage_extent); c; c.advance ())
    {
      auto& schunk = stage [c.x][c.y][c.z];
      loaded [c.x][c.y][c.z] = schunk.chunk && schunk.chunk -> loaded;
    }*/

    v3i v = stage_extent - v3i (1, 1, 1); // why does this work
    for (auto c = iteration (v3i (1, 1, 1), v); c; c.advance ())
    {
      //v3i cp = (stage_base + c) % stage_dim;
      auto& schunk = stage_at (c);

      if (!schunk.chunk || !schunk.chunk -> loaded)
        continue;

      if (schunk.chunk -> dirty)
      {
        const Chunk* neighbours [6] = {
          stage_at (c + v3i ( 1,  0,  0)).chunk.get (),
          stage_at (c + v3i (-1,  0,  0)).chunk.get (),
          stage_at (c + v3i ( 0,  1,  0)).chunk.get (),
          stage_at (c + v3i ( 0, -1,  0)).chunk.get (),
          stage_at (c + v3i ( 0,  0,  1)).chunk.get (),
          stage_at (c + v3i ( 0,  0, -1)).chunk.get ()
        };

        bool regen = true;

        for (uint i = 0; i != 6; i++)
        {
          if (!neighbours [i] || !neighbours [i] -> loaded)
            regen = false;
        }

        if (regen)
        {
          schunk.mesh.regen (*schunk.chunk, neighbours);
          schunk.chunk -> dirty = false;
        }
        else
        {
          continue;
        }
      }

      // Frustum cull chunk
      bool draw = true;

      for (uint i = 0; i != 4; i++)
      {
        v3f bp = schunk.chunk -> bpos;
        v3f centre = bp + (v3i (chunk_dim, chunk_dim, chunk_dim) / 2) - view.position;
        static const float radius = float (chunk_dim / 2) * 1.73205f; // 8rt3
        if (dot (centre, planes [i]) < -radius)
        {
          draw = false;
          break;
        }
      }

      if (draw)
        schunk.mesh.draw (frame, schunk.chunk -> bpos, material);
    }

    frame.set_skybox (
      sky_tex -> get (),
      v3f (0.0f, 0.0f, 0.0f),
      1.0f
    );
  }

  World::World (Co::WorkQueue& queue, Co::RenderContext& rc, const Co::PropMap* props)
  {
    stage_base = v3i (0, 0, 0);

    seed = 0xfeedbeef;

    texture = texture_factory -> create (queue, "blocks.cotexture", false, false, false);
    sky_tex = texture_factory -> create (queue, "title.cotexture",  false, true, true);

    for (auto c = iteration (v3i (0, 0, 0), stage_extent); c; c.advance ())
      stage [c.x][c.y][c.z].mesh.init (queue, rc);
  }

  World::StageChunk& World::stage_at (v3i cv)
  {
    cv = (stage_base + cv) % stage_dim;
    if (cv.x < 0) cv.x += stage_dim;
    if (cv.y < 0) cv.y += stage_dim;
    if (cv.z < 0) cv.z += stage_dim;
    return stage [cv.x][cv.y][cv.z];
  }

  /*Co::Texture::Ptr World::sky_tex,
                   World::texture;*/

  Co::Entity::Ptr create_world (Co::WorkQueue& queue, Co::RenderContext& rc, const Co::PropMap* props)
  {
    return queue.gc_attach (new World (queue, rc, props));
  }

} // namespace SH
