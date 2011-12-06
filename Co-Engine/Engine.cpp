//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxEngine.hpp>

// Uses
#include <Co/IxEntityClass.hpp>
#include <Co/IxRenderer.hpp>
#include <Co/IxEntity.hpp>
#include <Co/IxLoader.hpp>
#include <Co/IxModule.hpp>
#include <Co/IxFrame.hpp>
#include <Co/IxGame.hpp>
#include <Co/Clock.hpp>

#include <Rk/VirtualOutStream.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Expose.hpp>
#include <Rk/Module.hpp>
#include <Rk/Mutex.hpp>

#include <unordered_map>
//#include <algorithm>
//#include <iterator>
#include <vector>

extern "C"
{
  __declspec(dllimport) void __stdcall Sleep (u32);
}

namespace Co
{
  class IxInputSink;
  
namespace
{
  //
  // = Module ==========================================================================================================
  //
  class Module :
    private Rk::Module,
    public  IxModule
  {
    long ref_count;

  public:
    Module (Rk::StringRef name) :
      Rk::Module (name), // loading from /Cobalt/Binaries is fine
      ref_count  (1)
    { }
    
    uint references () const
    {
      return uint (ref_count);
    }

    virtual void acquire ()
    {
      ref_count++;
    }

    virtual void release ()
    {
      ref_count--;
    }

    virtual void* expose (u64 ixid)
    {
      return Rk::Module::expose (ixid);
    }

  };

  //
  // = Engine ==========================================================================================================
  //
  class Engine :
    public IxEngine
  {
    // Subsystems
    IxGame*                    game;
    IxRenderer*                renderer;
    IxLoader*                  loader;
    Clock*                     clock;
    Rk::VirtualLockedOutStream log;

    // Input
    IxInputSink* input_sink;
    int          mouse_x,
                 mouse_y;

    // Timing
    static const float frame_rate,
                       frame_interval;
    float              time,
                       prev_time;

    // Termination
    bool running;
    
    // Type registry
    typedef std::unordered_map <Rk::StringRef, const IxEntityClass*> Registry;
    Registry registry;

    // Entities
    typedef std::vector <IxEntity*> EntityList;
    EntityList entities;

    // Module cache
    typedef std::unordered_map <Rk::ShortString <512>, Module*> ModuleCache;
    ModuleCache modules;
    
    // Initialization
    virtual bool init (IxRenderer* renderer_in, IxLoader* loader_in, Clock* clock_in, Rk::IxLockedOutStreamImpl* log_impl);
    virtual void destroy ();

    // Type registration
    virtual bool register_classes   (const IxEntityClass* const* classes_in, const IxEntityClass* const* end);
    virtual bool unregister_classes (const IxEntityClass* const* classes_in, const IxEntityClass* const* end);

    // Module loading
    virtual IxModule* load_module   (Rk::StringRef name);
    virtual u32       clear_modules ();

    // Object creation
    virtual IxEntity* create_entity (Rk::StringRef type, IxPropMap* props);
    virtual bool      open          (Rk::StringRef scene);

    // Simulation control
    virtual bool start     (float& new_time);
    virtual void wait      ();
    virtual bool update    (float& next_update);
    virtual void terminate ();

    // Game registration
    virtual void game_starting (IxGame* game);
    virtual void game_stopping (IxGame* game);

  public:
    void expose (void** out, u64 ixid);

  } engine;

  const float Engine::frame_rate     = 75.0f,
              Engine::frame_interval = 1.0f / frame_rate;

  bool Engine::init (IxRenderer* renderer_in, IxLoader* loader_in, Clock* clock_in, Rk::IxLockedOutStreamImpl* log_impl) try
  {
    if (!renderer_in || !loader_in || !clock_in || !log_impl)
      throw Rk::Exception ("Co-Engine: IxEngine::init - null pointer");

    if (running)
      throw Rk::Exception ("Co-Engine: IxEngine::init - engine is running");

    log.set_impl (log_impl);
    log << "- Engine initializing\n";

    renderer   = renderer_in;
    loader     = loader_in;
    clock      = clock_in;
    input_sink = 0;
    running    = false;

    return true;
  }
  catch (...)
  {
    return false;
  }

  void Engine::destroy ()
  {
    log << "- Engine shutting down\n";

    uint mod_count = clear_modules ();
    if (mod_count != 0 && log)
    {
      auto lock = log.get_lock ();
      lock << "! Co-Engine: IxEngine::destroy - " << mod_count << " modules still loaded:\n";
      for (auto iter = modules.begin (); iter != modules.end (); iter++)
        lock << "!   " << iter -> first << " (" << iter -> second -> references () << " references)\n";
    }

    if (!registry.empty ())
    {
      auto lock = log.get_lock ();
      lock << "! Co-Engine: IxEngine::destroy - " << registry.size () << " classes still registered:\n";
      for (auto iter = registry.begin (); iter != registry.end (); iter++)
        lock << "!   " << iter -> first << '\n';
    }
  }

  bool Engine::register_classes (const IxEntityClass* const* begin, const IxEntityClass* const* end) try
  {
    if (!begin || !end) throw Rk::Exception ("Co-Engine: IxEngine::register_classes - null pointer");
    if (end < begin)    throw Rk::Exception ("Co-Engine: IxEngine::register_classes - invalid range");
    
    while (begin != end)
    {
      registry.insert (std::make_pair ((*begin) -> name, *begin));
      begin++;
    }

    return true;
  }
  catch (...)
  {
    return false;
  }

  bool Engine::unregister_classes (const IxEntityClass* const* begin, const IxEntityClass* const* end) try
  {
    if (!begin || !end) throw Rk::Exception ("Co-Engine: IxEngine::unregister_classes - null pointer");
    if (end < begin)    throw Rk::Exception ("Co-Engine: IxEngine::unregister_classes - invalid range");

    while (begin != end)
      registry.erase ((*begin++) -> name);

    return true;
  }
  catch (...)
  {
    return false;
  }

  IxModule* Engine::load_module (Rk::StringRef name) try
  {
    if (!name)
      throw Rk::Exception ("Co-Engine: IxEngine::load_module - name is nil");

    Module* module;

    Rk::ShortString <512> tagged_name (name);
    tagged_name += CO_SUFFIX ".dll";

    auto iter = modules.find (tagged_name);
    if (iter != modules.end ())
    {
      module = iter -> second;
      module -> acquire ();
    }
    else
    {
      module = new Module (tagged_name);
      modules.insert (std::make_pair (tagged_name, module));
    }
    
    return module;
  }
  catch (...)
  {
    return 0;
  }

  u32 Engine::clear_modules ()
  {
    u32 count = 0;

    for (auto module = modules.begin (); module != modules.end ();)
    {
      if (module -> second -> references () == 0)
      {
        module = modules.erase (module);
      }
      else
      {
        count++;
        module++;
      }
    }

    return count;
  }

  IxEntity* Engine::create_entity (Rk::StringRef type, IxPropMap* props) try
  {
    if (!type)
      throw Rk::Exception ("Co-Engine: IxEngine::create_entity - type is nil");

    auto iter = registry.find (type);
    if (iter == registry.end ())
      throw Rk::Exception () << "Co-Engine: IxEngine::create_entity - no such entity class \"" << type << "\"";

    auto ent = iter -> second -> create (loader, props);
    entities.push_back (ent);
    return ent;
  }
  catch (...)
  {
    return 0;
  }

  bool Engine::open (Rk::StringRef scene)
  {
    return false;
  }

  bool Engine::start (float& new_time) try
  {
    if (running)
      throw Rk::Exception ("Co-Engine: IxEngine::start - engine already running");

    log << "- Engine starting\n";

    running = true;
    prev_time = clock -> time ();
    time = prev_time + frame_interval;
    new_time = time;
    return true;
  }
  catch (...)
  {
    return false;
  }

  void Engine::wait ()
  {
    float now = clock -> time ();
    if (now < time)
    {
      if (time - now >= 0.003f)
      {
        uint delay = uint ((time - now - 0.002f) * 0.001f);
        if (delay) Sleep (delay);
      }
      
      while (clock -> time () < time) { }
    }
  }

  bool Engine::update (float& next_update)
  {
    if (!running)
      return false;

    //prev_time = clock -> time ();
    //time = prev_time + frame_interval;
    
    //int old_mouse_x = 0, old_mouse_y = 0;
    
    u32 recycled_id, frame_id;
    IxFrame* frame = renderer -> begin_frame (prev_time, time, recycled_id, frame_id);
    
    frame -> set_size (1280, 720);
    
    /*if (scene)
      scene -> draw ();*/
    
    /*if (cube_world)
      cube_world -> draw ();*/
    
    /*if ((mouse_x != old_mouse_x || mouse_y != old_mouse_y) && mouse_mode == mouse_cursor)
    {
      input_handler -> mouse_move (mouse_x, mouse_y);
      old_mouse_x = mouse_x;
      old_mouse_y = mouse_y;
    }
    else if (mouse_mode == mouse_fps)
    {
      Point mouse_pos;
      GetCursorPos (&mouse_pos);
      input_handler -> mouse_move (mouse_pos.x - mid_x, mouse_pos.y - mid_y);
      SetCursorPos (mid_x, mid_y);
    }*/
    
    //auto current_ui_state = ui_state;
    
    //game -> tick (time, prev_time);
    
    /*if (current_ui_state == ui_enabled)
      game -> update_ui (ui);
    
    if (current_ui_state != ui_hidden && frame)
      ui.paint (frame);*/
    
    game -> tick (frame, time, prev_time);

    for (auto ent = entities.begin (); ent != entities.end (); ent++)
      (*ent) -> tick (frame, time, prev_time);
    
    renderer -> submit_frame (frame);

    prev_time = time;
    time      += frame_interval;

    return true;
  } // update

  void Engine::terminate ()
  {
    log << "- Engine stopping\n";
    running = false;
  }

  void Engine::game_starting (IxGame* new_game)
  {
    game = new_game;
  }

  void Engine::game_stopping (IxGame* new_game)
  {
    game = 0;
  }

  void Engine::expose (void** out, u64 ixid)
  {
    Rk::expose <Co::IxEngine> (this, ixid, out);
  }

} // namespace
  
  IX_EXPOSE (void** out, u64 ixid)
  {
    engine.expose (out, ixid);
  }

} // namespace Co
