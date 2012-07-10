//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENT_H_COMMON
#define CO_CLIENT_H_COMMON

#include <Co/Log.hpp>

#include <Rk/Module.hpp>

namespace Co
{
  extern Log log;

  Rk::Module load_module (Rk::StringRef path);

}

#endif
