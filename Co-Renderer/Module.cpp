//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Rk/Expose.hpp>

#include "GLRenderer.hpp"

namespace Co
{
  //
  // Module interface
  //
  IX_EXPOSE (void** out, u64 ixid)
  {
    renderer.expose (ixid, out);
  }

}
