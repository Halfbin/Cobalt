//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/Library.hpp>

#include <Rk/Expose.hpp>
#include <Rk/Memory.hpp>

namespace SH
{
  extern Co::IxEntityClass *test_class,
                           *block_world_class,
                           *spectator_class;

  namespace
  {
    Co::IxEntityClass* classes [] = {
      test_class,
      block_world_class,
      spectator_class
    };

    Co::Library lib (classes);

  }

  void expose_lib (u64 ixid, void** out)
  {
    lib.expose (out, ixid);
  }

}
