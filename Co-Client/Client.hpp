//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENT_H_CLIENT
#define CO_CLIENT_H_CLIENT

#include <Co/Filesystem.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/Renderer.hpp>
#include <Co/UIInput.hpp>
#include <Co/Engine.hpp>
#include <Co/Clock.hpp>
#include <Co/Host.hpp>
#include <Co/Log.hpp>

#include <Rk/StringRef.hpp>
#include <Rk/Module.hpp>
#include <Rk/Thread.hpp>
#include <Rk/Types.hpp>

#include "GLWindow.hpp"

#include <memory>
#include <vector>
#include <map>

namespace Co
{
  class RenderDevice;

  class Client :
    public Host
  {
    WorkQueue::Ptr           queue;
    std::vector <Rk::Thread> pool;
    GLWindow                 window;
    MasterClock              clock;
    Renderer::Ptr            renderer;
    Engine::Ptr              engine;
    //Filesystem::Ptr          filesystem;
    
    std::map <std::string, std::string>
      module_config;

    std::map <std::string, std::shared_ptr <void>>
      objects;

    KeyState              keyboard [key_count];
    bool                  ui_enabled;
    std::vector <UIEvent> ui_events;

    void worker (RenderDevice& device, Filesystem& fs);

    static iptr handler_proxy (GLWindow*, u32, uptr, iptr);
    iptr        handler       (u32, uptr, iptr);

    void update_keyboard ();
    v2f  update_mouse ();

    virtual std::shared_ptr <void> get_object (Rk::StringRef type);

    template <typename T>
    std::shared_ptr <T> get_object ()
    {
      return std::static_pointer_cast <T> (
        get_object (T::ix_name ())
      );
    }

    template <typename T>
    void get_object (std::shared_ptr <T>& ptr)
    {
      ptr = get_object <T> ();
    }

    virtual void enable_ui (bool enabled);

  public:
    Client (Rk::StringRef config_path);
    ~Client ();

    void run ();

  }; // class Client

} // namespace Co

#endif
