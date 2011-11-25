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
#include <Co/Library.hpp>
#include <Co/IxGame.hpp>

#include <Rk/ShortString.hpp>

#include "GLWindow.hpp"
#include "Common.hpp"

struct Point
{
  i32 x, y;
};

struct Message
{
  void* window;
  u32   message;
  uptr  wp;
  iptr  lp;
  u32   time;
  Point cursor;
};

extern "C"
{
  __declspec(dllimport) void __stdcall PostQuitMessage  (u32);
  __declspec(dllimport) i32  __stdcall PeekMessageW     (Message*, void*, u32, u32, u32);
  __declspec(dllimport) i32  __stdcall DispatchMessageW (const Message*);
}

enum WinAPIConstants
{
  wm_size  = 0x05,
  wm_close = 0x10
};

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
        engine -> terminate ();
      break;

      case wm_size:
        render_device -> set_size (lp & 0xffff, (lp >> 16) & 0xffff);
      break;

      default:
        return window.message_default (message, wp, lp);
    }

    return 0;
  }

  Client::Client (Rk::StringRef config_path)
  {
    // Parse configuration
    //ConfigReader config (config_path);
    Rk::ShortString <512> game_path ("../");
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
    game    = game_module.expose <IxGame>  ();
    library = game_module.expose <Library> ();

    // Initialize subsystems
    window.create (L"Cobalt", handler_proxy, false, 1280, 720, this);
    
    renderer -> init (window.get_handle (), clock, log);
    loader   -> init (*render_device, log, game_path);
    engine   -> init (*renderer, *loader, clock);
    game     -> init (*engine, &log);

    engine -> register_classes (library -> classes, library -> class_count);
  }

  Client::~Client ()
  {
    window.hide ();
    game     -> stop ();
    renderer -> stop ();
    loader   -> stop ();
  }

  void Client::run ()
  {
    loader   -> start ();
    renderer -> start ();
    game     -> start (*engine);

    window.show ();

    float next_update = engine -> start ();

    for (;;)
    {
      Message msg;
      while (PeekMessageW (&msg, 0, 0, 0, 1))
      {
        DispatchMessageW (&msg);
        if (clock.time () + 0.01 >= next_update)
          break;
      }

      engine -> wait ();

      if (!engine -> update (next_update))
        break;
    }
  } // run

} // namespace Co
