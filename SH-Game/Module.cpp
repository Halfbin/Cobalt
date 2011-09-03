//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Rk/Expose.hpp>

void expose_game (u64, void**);
void expose_lib  (u64, void**);

IX_EXPOSE (void** out, u64 ixid)
{
  expose_game (ixid, out);
  expose_lib  (ixid, out);
}
