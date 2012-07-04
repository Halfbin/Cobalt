//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Engine.hpp>
#include <Co/Module.hpp>

// Uses
#include <Co/EntityClass.hpp>
#include <Co/InputSink.hpp>

#include <Rk/Module.hpp>

#include <vector>

extern "C"
{
  __declspec(dllimport) void __stdcall Sleep (u32);
}

namespace Co
{
  namespace
  {
    //
    // = EngineImpl ====================================================================================================
    //
    class EngineImpl :
      public Engine
    {
      // Subsystems
      Host*         host;
      Renderer::Ptr renderer;
      WorkQueue*    queue;
      Game::Ptr     game;
      
      float get_time ()
      {
        return host -> clock.time ();
      }

      Log::Lock log ()
      {
        return host -> log ();
      }

      // Input
      InputSink::Ptr input_sink;
      int            mouse_x,
                     mouse_y;

      // Timing
      static const float frame_rate,
                         frame_interval;
      float              time,
                         prev_time;

      // Termination
      bool running;
      
      // Entities
      std::vector <Entity::Ptr> entities;

      // Simulation control
      virtual void  init        (Host&, Renderer::Ptr, WorkQueue&, Game::Ptr);
      virtual float start       ();
      virtual void  post_events (const UIEvent* events, const UIEvent* end);
      virtual void  wait        ();
      virtual bool  update      (float&, uint, uint, const UIEvent*, uint, const KeyState*, v2f);
      virtual void  stop        ();
      virtual void  enable_ui   (bool enable);

      // Object creation
      virtual Entity::Ptr create_entity (Rk::StringRef type, const PropMap* props);
      
      // Linking
      virtual std::shared_ptr <void> get_object (Rk::StringRef type);

    public:
      EngineImpl () :
        running (false)
      { } 
      
      static Ptr create ()
      {
        return std::make_shared <EngineImpl> ();
      }
      
    };

    RK_MODULE_FACTORY (EngineImpl);

    const float EngineImpl::frame_rate     = 60.0f,
                EngineImpl::frame_interval = 1.0f / frame_rate;

    void EngineImpl::init (Host& new_host, Renderer::Ptr new_renderer, WorkQueue& new_queue, Game::Ptr new_game)
    {
      if (!new_renderer || !new_game)
        throw std::invalid_argument ("Null pointer");
      if (running)
        throw std::logic_error ("Engine is running");

      host = &new_host;
      
      log () << "- Engine initializing\n";

      renderer   = new_renderer;
      queue      = &new_queue;
      game       = new_game;
      input_sink = 0;
      running    = false;

      game -> init (*this, *queue, host -> log);
    }

    float EngineImpl::start () 
    {
      if (running)
        throw std::logic_error ("Engine already running");

      log () << "- Engine starting\n";

      game -> start (*this);

      running = true;
      prev_time = get_time ();
      time = prev_time + frame_interval;
      return time;
    }

    void EngineImpl::wait ()
    {
      float now = get_time ();

      if (now < time)
      {
        if (time - now >= 0.003f)
        {
          uint delay = uint ((time - now - 0.002f) * 0.001f);
          if (delay) Sleep (delay);
        }
        
        while (get_time () < time) { }
      }
    }

    bool EngineImpl::update (float& next_update, uint width, uint height, const UIEvent* ui_events, uint ui_event_count, const KeyState* keyboard, v2f mouse_delta)
    {
      if (!running)
      {
        game -> stop ();
        return false;
      }

      Frame* frame = renderer -> begin_frame (prev_time, time);
      
      frame -> set_size (width, height);
      
      game -> tick (*queue, *frame, time, prev_time, ui_events, ui_event_count);

      for (auto ent = entities.begin (); ent != entities.end (); ent++)
        (*ent) -> tick (*frame, time, prev_time, keyboard, mouse_delta);
      
      frame -> submit ();

      prev_time = time;
      time      += frame_interval;

      return true;
    }

    void EngineImpl::stop ()
    {
      log () << "- Engine stopping\n";
      running = false;
    }

    void EngineImpl::enable_ui (bool enable)
    {
      host -> enable_ui (enable);
    }

    void EngineImpl::post_events (const UIEvent* begin, const UIEvent* end)
    {
      if (!begin || end < begin)
        throw std::invalid_argument ("Invalid range");
    }

    Entity::Ptr EngineImpl::create_entity (Rk::StringRef class_name, const PropMap* props)
    {
      if (!class_name)
        throw std::invalid_argument ("class_name is nil");

      auto& factory = game -> find_class (class_name);
      auto ent = factory.create (*queue, props);
      entities.push_back (ent);
      return std::move (ent);
    }

    std::shared_ptr <void> EngineImpl::get_object (Rk::StringRef type)
    {
      return host -> get_object (type);
    }

  } // namespace

} // namespace Co
