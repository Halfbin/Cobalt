//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENT_H_CLIENT
#define CO_CLIENT_H_CLIENT

#include "GLWindow.hpp"
#include <Co/Clock.hpp>
#include <Rk/Module.hpp>
#include <Rk/Types.hpp>
#include <Rk/StringRef.hpp>

namespace Co
{
  class IxRenderer;
  class IxRenderDevice;
  class IxLoader;
  class IxEngine;
  class IxGame;

  class Client
  {
    GLWindow        window;
    Clock           clock;

    Rk::Module      renderer_module;
    IxRenderer*     renderer;
    IxRenderDevice* render_device;

    Rk::Module      loader_module;
    IxLoader*       loader;

    Rk::Module      engine_module;
    IxEngine*       engine;

    Rk::Module      game_module;
    IxGame*         game;

    void cleanup ();

    static iptr handler_proxy (GLWindow*, u32, uptr, iptr);
    iptr        handler       (u32, uptr, iptr);

  public:
    Client (Rk::StringRef config_path);
    ~Client ();

    void run ();

  };

}

#endif
