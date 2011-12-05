//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENT_H_CLIENT
#define CO_CLIENT_H_CLIENT

#include <Co/IxEngine.hpp>
#include <Co/IxGame.hpp>
#include <Co/Clock.hpp>

#include <Rk/StringRef.hpp>
#include <Rk/Module.hpp>
#include <Rk/Types.hpp>

#include "GLWindow.hpp"

namespace Co
{
  class IxRenderer;
  class IxRenderDevice;
  class IxLoader;
  class IxGame;
  class Library;
  class GLWindow;
  
  class Client
  {
    GLWindow    window;
    MasterClock clock;

    Rk::Module renderer_module,
               loader_module,
               engine_module,
               game_module;

    IxRenderer*     renderer;
    IxRenderDevice* render_device;
    IxLoader*       loader;
    IxEngine::Ptr   engine;
    IxGame::Ptr     game;
    Library*        library;

    void cleanup ();

    static iptr handler_proxy (GLWindow*, u32, uptr, iptr);
    iptr        handler       (u32, uptr, iptr);

  public:
    Client (Rk::StringRef config_path);
    ~Client ();

    void run ();

  }; // class Client

} // namespace Co

#endif
