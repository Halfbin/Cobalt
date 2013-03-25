//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENTFRONTEND_H_CLIENTFRONTEND
#define CO_CLIENTFRONTEND_H_CLIENTFRONTEND

// Uses
#include <Co/ModuleManager.hpp>
#include <Co/ClientWindow.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/Renderer.hpp>
#include <Co/UIInput.hpp>
#include <Co/Clock.hpp>
#include <Co/Game.hpp>
#include <Co/Log.hpp>

#include <Rk/MethodProxy.hpp>

#include <vector>

namespace Co
{
  class ClientFrontend
  {
    std::string     exe_name;
    LogFile         log_file;
    Rk::OutStream   log_str;
    Log             log;
    ModuleManager   modman;
    ClientWindow    window;
    MasterClock     clock;
    WorkQueue::Ptr  queue;
    Renderer::Ptr   renderer;
    GameServer::Ptr server,
                    next_server;
    GameClient::Ptr client,
                    next_client;
    bool            running;

    // UI
    bool                  ui_enabled,
                          prev_ui_enabled;
    KeyState              keyboard [key_count];
    std::vector <UIEvent> ui_events;

    void run_loop (GameClient& gc);

    iptr handle_message (ClientWindow*, u32 message, uptr wp, iptr lp);

    void update_keyboard ();
    v2f  update_mouse    ();

  public:
    ClientFrontend (Rk::StringRef new_exe_name, Rk::WStringRef app_name, bool fullscreen, uint w, uint h);

    ~ClientFrontend ()
    {
      log () << "* " << exe_name << " exiting\n";
    }

    void show ()
    {
      window.show ();
    }

    template <typename T, typename Params>
    std::shared_ptr <T> load_module (Rk::StringRef img_name, const Params& params) try
    {
      return modman.load (img_name, params).get <T> ();
    }
    catch (...)
    {
      log_exception (log, "during module load");
      throw;
    }

    template <typename T, typename Params>
    void load_module (std::shared_ptr <T>& ptr, Rk::StringRef img_name, const Params& params)
    {
      ptr = load_module <T> (img_name, params);
    }

    /*GameClient::Ptr load_client (Rk::StringRef img_name)
    {
      return load_module <GameClient> (img_name, 0);
    }

    GameServer::Ptr load_server (Rk::StringRef img_name)
    {
      return load_module <GameServer> (img_name, 0);
    }*/

    void begin_game (GameClient::Ptr new_client, GameServer::Ptr new_server)
    {
      next_client = new_client;
      next_server = new_server;
    }

    void run (GameClient::Ptr gc);

    void enable_ui (bool new_enabled);

  };

}

#endif
