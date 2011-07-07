//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "Client.hpp"

// Uses
#include <Co/IxRenderDevice.hpp>
#include <Co/IxRenderer.hpp>
#include <Co/IxEngine.hpp>
#include <Co/IxLoader.hpp>
#include <Co/IxGame.hpp>

#include <Rk/ShortString.hpp>

namespace
{
  extern "C"
  {
    __declspec(dllimport) void __stdcall PostQuitMessage (u32);
  }

  enum
  {
    wm_close = 0x10
  };
}

namespace Co
{
  iptr Client::handler_proxy (GLWindow* win, u32 message, uptr wp, iptr lp)
  {
    auto client = (Client*) win -> get_user ();
    return client -> handler (message, wp, lp);
  }

  iptr Client::handler (u32 message, uptr wp, iptr lp)
  {
    switch (message)
    {
      case wm_close:
        PostQuitMessage (0);
      break;

      default:
        return window.message_default (message, wp, lp);
    }

    return 0;
  }

  Client::Client (Rk::StringRef config_path) try
  {
    // Parse configuration
    //ConfigReader config (config_path);
    Rk::ShortString <256> game_path ("../");
    game_path += "SH/";//config ["Client.Game"].as_string ("SH");

    // Load subsystem modules
    renderer_module.load ("Co-GLRenderer" CO_SUFFIX ".dll");
    renderer      = renderer_module.expose <IxRenderer>     ();
    render_device = renderer_module.expose <IxRenderDevice> ();

    loader_module.load ("Co-Loader" CO_SUFFIX ".dll");
    loader = loader_module.expose <IxLoader> ();

    engine_module.load ("Co-Engine" CO_SUFFIX ".dll");
    engine = engine_module.expose <IxEngine> ();

    game_module.load (game_path + "Binaries/Co-Game" CO_SUFFIX ".dll");
    game = game_module.expose <IxGame> ();

    // Initialize subsystems
    window.create (L"Cobalt", handler_proxy, false, 1280, 720, this);

    renderer -> init (window.get_handle (), &clock);
    loader   -> init (render_device);
    engine   -> init (renderer, loader, &clock);
    game     -> init (engine);
  }
  catch (...)
  {
    Rk::log_frame ("Co::Client::Client")
      << " config_path: " << config_path;
  }

  Client::~Client ()
  {
    window.hide ();
    game     -> stop ();
    renderer -> stop ();
    loader   -> stop ();
  }

  void Client::run () try
  {
    loader   -> start ();
    renderer -> start ();
    game     -> start ();
    window.show ();
    engine -> run (game);
  }
  catch (...)
  {
    Rk::log_frame ("Co::Client::run");
    throw;
  }

} // namespace Co
