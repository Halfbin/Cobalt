//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include 

//
  // Module interface
  //
  extern "C" __declspec(dllexport) void* __cdecl ix_expose (u64 ixid)
  {
    switch (ixid)
    {
      case IxRenderer::id:
      case IxRenderDevice::id:
        return &renderer;

      default:
        return 0;
    }
  }