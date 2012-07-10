//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef IR_H_COMMON
#define IR_H_COMMON

#include <Co/Texture.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>
#include <Co/Log.hpp>

namespace Ir
{
  extern Co::Log* log_ptr;

  static inline Co::Log::Lock log ()
  {
    return log_ptr -> lock ();
  }

  extern Co::TextureFactory::Ptr texture_factory;
  extern Co::ModelFactory::Ptr   model_factory;
  extern Co::FontFactory::Ptr    font_factory;
  
}

#endif
