//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/IxGame.hpp>

extern Co::IxGame* ix_game;

extern "C" __declspec(dllexport) void* __cdecl ix_expose (u64 ixid)
{
  if (ixid == Co::IxGame::id) return ix_game;
  else return 0;
}
