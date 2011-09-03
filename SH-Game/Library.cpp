//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/Library.hpp>

#include <Rk/Expose.hpp>
#include <Rk/Memory.hpp>

extern Co::IxEntityClass* test_class;

namespace
{
  Co::IxEntityClass* classes [] = {
    test_class
  };

  Co::Library lib (classes);

}

void expose_lib (u64 ixid, void** out)
{
  Rk::expose <Co::Library> (&lib, ixid, out);
}
