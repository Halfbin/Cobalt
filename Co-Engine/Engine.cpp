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
#include <Co/IxGame.hpp>
#include <Co/Clock.hpp>
#include <Co/Frame.hpp>

#include <Rk/Exception.hpp>
#include <Rk/Expose.hpp>
#include <Rk/Module.hpp>
#include <Rk/Mutex.hpp>

#include <unordered_map>
#include <deque>

extern "C"
{
  __declspec(dllimport) void __stdcall Sleep (u32);
}

namespace Co
{
  class IxInputSink;
  class Frame;
  
namespace
{
  //
  // = Module ==========================================================================================================
  //
  class Module :
    private Rk::Module,
    public  IxModule
  {
    virtual void* expose (u64 ixid)
    {
      return Rk::Module::expose (ixid);
    }

  public:
    Module (Rk::StringRef name) :
      Rk::Module (name) // loading from /Cobalt/Binaries is fine
    { }
    
  };

  //
  // = Engine ==========================================================================================================
  //
  class Engine :
    public IxEngine
  {
    // Subsystems
    IxRenderer* renderer;
    IxLoader*   loader;
    Clock*      clock;
    
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
    typedef std::unordered_map <Rk::StringRef, IxEntityClass*> Registry;
    Registry registry;

    // Entities
    typedef std::deque <IxEntity*> EntityList;
    EntityList entities;

    // Initialization and cleanup
    virtual void init (IxRenderer& renderer_in, IxLoader& loader_in, Clock& clock_in);
    void cleanup ();

    // Type registration
    virtual void register_classes   (IxEntityClass** classes_in, uint count);
    virtual void unregister_classes (IxEntityClass** classes_in, uint count);

    // Module loading
    virtual IxModule* load_module (Rk::StringRef name);

    // Object creation
    virtual IxEntity* create_entity (Rk::StringRef type, IxPropMap* props);
    virtual void      open          (Rk::StringRef scene);

    // Simulation control
    virtual float start     ();
    virtual void  wait      ();
    virtual bool  update    (float& next_update);
    virtual void  terminate ();

  } engine;

  const float Engine::frame_rate     = 50.0f,
              Engine::frame_interval = 1.0f / frame_rate;

  void Engine::init (IxRenderer& renderer_in, IxLoader& loader_in, Clock& clock_in)
  {
    Rk::require (!running, "Attempt to initialize engine while it is running");
    
    renderer   = &renderer_in;
    loader     = &loader_in;
    clock      = &clock_in;
    input_sink = 0;
    running    = false;
  }
  
  void Engine::cleanup ()
  {

  }

  void Engine::register_classes (IxEntityClass** classes_in, uint count)
  {
    Rk::require (classes_in != 0, "register_classes: classes is null");
    Rk::require (count != 0,      "register_classes: count is 0");
    
    for (auto p = classes_in; p != classes_in + count; p++)
      registry.insert (Registry::value_type ((*p) -> name, *p));
  }

  void Engine::unregister_classes (IxEntityClass** classes_in, uint count)
  {
    Rk::require (classes_in != 0, "unregister_classes: classes is null");
    Rk::require (count != 0,      "unregister_classes: count is 0");

    for (auto p = classes_in; p != classes_in + count; p++)
      registry.erase ((*p) -> name);
  }

  IxModule* Engine::load_module (Rk::StringRef name)
  {
    Rk::ShortString <512> tagged_name (name);
    tagged_name += CO_SUFFIX ".dll";
    return new Module (tagged_name);
  }

  IxEntity* Engine::create_entity (Rk::StringRef type, IxPropMap* props)
  {
    Rk::require (type, "type is nil");

    auto iter = registry.find (type);
    if (iter == registry.end ())
      throw Rk::Exception ("No such entity class");

    auto ent = iter -> second -> create (*loader, props);
    entities.push_back (ent);
    return ent;
  }

  void Engine::open (Rk::StringRef scene)
  {

  }

  float Engine::start ()
  {
    running = true;
    prev_time = clock -> time ();
    time = prev_time + frame_interval;
    return time;
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
    Frame* frame = renderer -> begin_frame (prev_time, time, recycled_id, frame_id);
    
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
    
    for (auto ent = entities.begin (); ent != entities.end (); ent++)
      (*ent) -> tick (time, prev_time, *frame);
    
    renderer -> submit_frame (frame);

    prev_time = time;
    time      += frame_interval;

    return true;
  } // update

  void Engine::terminate ()
  {
    running = false;
  }

} // namespace
  
  IX_EXPOSE (void** out, u64 ixid)
  {
    Rk::expose <Co::IxEngine> (engine, ixid, out);
  }

} // namespace Co
