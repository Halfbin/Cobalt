//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef IR_H_COMMON
#define IR_H_COMMON

#include <Co/IxTextureFactory.hpp>
#include <Co/IxModelFactory.hpp>
#include <Co/IxFontFactory.hpp>
#include <Co/IxEngine.hpp>

namespace Ir
{
  extern Co::IxTextureFactory* texture_factory;
  extern Co::IxModelFactory*   model_factory;
  extern Co::IxFontFactory*    font_factory;
  extern Co::IxLoadContext*    load_context;
  extern Co::IxEngine*         engine;

}

#endif
