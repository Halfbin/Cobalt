//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_COMMON
#define SH_H_COMMON

#include <Co/PropMap.hpp>
#include <Co/Texture.hpp>
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

  extern Co::TextureFactory::Ptr texture_factory;
  extern Co::ModelFactory::Ptr   model_factory;
  extern Co::FontFactory::Ptr    font_factory;

  static const int
    chunk_dim          = 16, // must be power of two and thus even
    chunk_volume       = chunk_dim * chunk_dim * chunk_dim,
    chunk_max_faces    = chunk_volume * 3,
    chunk_max_vertices = chunk_max_faces * 4,
    chunk_max_indices  = chunk_max_faces * 6,

    stage_dim    = 15,           // should be odd
    stage_radius = stage_dim / 2 // rounds down
  ;

  static const v3i
    chunk_extent (chunk_dim, chunk_dim, chunk_dim),
    stage_extent (stage_dim, stage_dim, stage_dim)
  ;

  static inline uint bit_count (u16 word)
  {
    #if 1
      return (uint) __popcnt16 (short (word));
    #else
      u16 count = word;
      count = ((count >> 1) & 0x5555) + (count & 0x5555);
      count = ((count >> 2) & 0x3333) + (count & 0x3333);
      count = ((count >> 4) & 0x0f0f) + (count & 0x0f0f);
      count = ((count >> 8) & 0x00ff) + (count & 0x00ff);
      return count;
    #endif
  }

  static inline u16 rotr (u16 word, u8 dist)
  {
    #if 1
      return (u16) _rotr16 (ushort (word), uchar (dist));
    #else
      return (word >> dist) | (word << (16 - dist));
    #endif
  }

}

#endif
