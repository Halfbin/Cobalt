//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/IxEngine.hpp>
#include <Co/Clock.hpp>
#include <unordered_map>
#include <Rk/Exception.hpp>
#include <Co/IxEntityClass.hpp>
#include <Co/IxRenderer.hpp>
#include <Co/IxGame.hpp>
#include <list>
#include <Co/IxThinker.hpp>
#include <Co/Frame.hpp>

namespace
{
  struct Point
  {
    i32 x,
        y;
  };
  
  struct Message
  {
    void* hwnd;
    u32   message;
    uptr  wParam;
    iptr  lParam;
    u32   time;
    Point pt;
  };

  struct Rect
  {
    i32 left,
        top,
        right,
        bottom;
  };

  extern "C"
  {
    __declspec(dllimport) void __stdcall PostQuitMessage  (void*);
    __declspec(dllimport) iptr __stdcall DefWindowProcW   (void*, u32, uptr, iptr);
    __declspec(dllimport) i32  __stdcall PeekMessageW     (void*, Message*, u32, u32, u32);
    __declspec(dllimport) iptr __stdcall DispatchMessageW (Message*);
    __declspec(dllimport) void __stdcall Sleep            (u32);
    __declspec(dllimport) i32  __stdcall ShowCursor       (i32);
    __declspec(dllimport) i32  __stdcall SetCursorPos     (i32, i32);
    __declspec(dllimport) i32  __stdcall GetSystemMetrics (i32);
    __declspec(dllimport) i32  __stdcall ClipCursor       (const Rect*);
    __declspec(dllimport) i32  __stdcall GetCursorPos     (Point*);
  }

  enum
  {
    WM_QUIT        = 0x12,
    WM_CLOSE       = 0x10,
    WM_SIZE        = 0x05,
    WM_ACTIVATEAPP = 0x1C,
    //WM_SETFOCUS    = 0x07,
    //WM_KILLFOCUS   = 0x08,
    
    WM_KEYDOWN = 0x100,
    WM_KEYUP   = 0x101,
    
    WM_MOUSEMOVE   = 0x200,
    WM_LBUTTONDOWN = 0x201,
    WM_LBUTTONUP   = 0x202,
    WM_RBUTTONDOWN = 0x204,
    WM_RBUTTONUP   = 0x205,
    WM_MBUTTONDOWN = 0x207,
    WM_MBUTTONUP   = 0x208,
    
    PM_REMOVE = 1,
    
    SM_CXSCREEN = 0,
    SM_CYSCREEN = 1
  };

} // namespace

namespace Co
{
  class IxInputSink;
  class Frame;
  
namespace
{
  class Engine :
    public IxEngine
  {
    IxRenderer* renderer;
    IxLoader*   loader;
    Clock*      clock;
    
    IxInputSink* input_sink;

    Frame* frame;
    int    mouse_x,
           mouse_y;
    float  time,
           prev_time;

    typedef std::unordered_map <u64, IxEntityClass*> Registry;
    Registry registry;

    typedef std::list <IxThinker*> ThinkerList;
    ThinkerList thinkers;

    virtual void init (IxRenderer* renderer_in, IxLoader* loader_in, Clock* clock_in)
    {
      renderer = renderer_in;
      loader   = loader_in;
      clock    = clock_in;
    }
    
    virtual void register_classes (IxEntityClass* classes_in, uint count)
    {
      for (auto p = classes_in; p != classes_in + count; p++)
        registry.insert (Registry::value_type (p -> id, p));
    }

    virtual void unregister_classes (IxEntityClass* classes_in, uint count)
    {
      for (auto p = classes_in; p != classes_in + count; p++)
        registry.erase (p -> id);
    }

    virtual IxEntity* create_entity (u64 type, IxPropMap* props, IxEntity* owner) try
    {
      auto iter = registry.find (type);
      if (iter == registry.end ())
        throw Rk::Exception ("No such entity class");
      auto impl = iter -> second -> create (props, owner);
      return 0;
    }
    catch (...)
    {
      Rk::log_frame ("Co::Engine::create_entity")
        << " type: " << type;
      throw;
    }

    virtual void open (Rk::StringRef scene)
    {

    }

    virtual void run (IxGame* game)
    {
      const float frame_rate = 50.0f;
      const float frame_interval = 1.0f / frame_rate;
      
      prev_time = clock -> time ();
      time = prev_time + frame_interval;
      
      int old_mouse_x = 0, old_mouse_y = 0;
      
      for (;;)
      {
        Message message;
        while (PeekMessageW (&message, 0, 0, 0, PM_REMOVE))
        {
          if (message.message == WM_QUIT)
            return;
          
          DispatchMessageW (&message);

          if (clock -> time () > time - 0.005f)
            break;
        }
        
        u32 recycled_id, frame_id;
        frame = renderer -> begin_frame (prev_time, time, recycled_id, frame_id);
        
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
        
        for (auto thinker = thinkers.begin (); thinker != thinkers.end (); thinker++)
          (*thinker) -> think (time, prev_time);
        
        if (frame)
        {
          renderer -> submit_frame (frame);
          frame = 0;
        }
        
        float now = clock -> time ();
        if (now < time)
        {
          if (time - now >= 0.003f)
          {
            uint delay = (time - now - 0.002f) * 0.001f;
            if (delay) Sleep (delay);
          }
          
          while (clock -> time () < time) { }
        }
        
        prev_time = time;
        time      += frame_interval;
      }
    } // run

    virtual void terminate ()
    {
      PostQuitMessage (0);
    }

  };
  
  Engine engine;

} // namespace
  
  extern "C" __declspec(dllexport) void* __cdecl ix_expose (u64 ixid)
  {
    if (ixid == IxEngine::id) return &engine;
    else return 0;
  }

} // namespace Co
