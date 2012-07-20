//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_COMMON
#define SH_H_COMMON

#include <Co/PropMap.hpp>
#include <Co/Texture.hpp>
#include <Co/Entity.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>
#include <Co/Log.hpp>

namespace SH
{
  extern Co::Log* log_ptr;

  static inline Co::Log::Lock log ()
  {
    return log_ptr -> lock ();
  }

  typedef Co::Entity::Ptr (CreateEntFunc) (Co::WorkQueue&, Co::RenderContext& rc, const Co::PropMap*);

  CreateEntFunc
    create_spectator,
    create_world,
    create_test_entity;

  extern Co::TextureFactory::Ptr texture_factory;
  extern Co::ModelFactory::Ptr   model_factory;
  extern Co::FontFactory::Ptr    font_factory;
  
  extern Co::Spatial view_cur, view_next;

  static const int
    chunk_dim          = 16, // must be power of two and thus even
    chunk_max_faces    = chunk_dim * chunk_dim * chunk_dim * 3,
    chunk_max_vertices = chunk_max_faces * 4,
    chunk_max_indices  = chunk_max_faces * 6,

    stage_dim    = 15,           // should be odd
    stage_radius = stage_dim / 2 // rounds down
  ;

  static const v3i
    chunk_extent (chunk_dim, chunk_dim, chunk_dim),
    stage_extent (stage_dim, stage_dim, stage_dim)
  ;

}

#endif
