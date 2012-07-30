//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Engine.hpp>
#include <Co/Module.hpp>

// Uses
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
      v2f            mouse_motion;

      // Timing
      static const float tick_rate,
                         tick_interval;
      float              prev_real_time,
                         sim_time,
                         time_accumulator;

      // Termination
      bool running;
      
      // Entities
      std::vector <Entity::Ptr> entities;

      // Simulation control
      virtual void init      (Host&, Renderer::Ptr, WorkQueue&, Game::Ptr);
      virtual void start     ();
      virtual bool update    (uint, uint, const UIEvent*, uint, const KeyState*, v2f);
      virtual void stop      ();
      virtual void enable_ui (bool enable);

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

    const float EngineImpl::tick_rate     = 50.0f,
                EngineImpl::tick_interval = 1.0f / tick_rate;

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

      game -> init (*this, *queue, renderer -> context (), host -> log);
    }

    void EngineImpl::start () 
    {
      if (running)
        throw std::logic_error ("Engine already running");

      game -> start (*this);

      log () << "* Engine starting\n";

      running = true;
      prev_real_time   = get_time ();
      sim_time         = 0.0f;
      time_accumulator = 0.0f;

      mouse_motion = v2f (0, 0);
    }

    bool EngineImpl::update (uint width, uint height, const UIEvent* ui_events, uint ui_event_count, const KeyState* keyboard, v2f mouse_delta)
    {
      if (!running)
      {
        game -> stop ();
        return false;
      }

      queue -> do_completions ();

      mouse_motion += mouse_delta;
      mouse_delta = mouse_motion / time_accumulator;

      while (time_accumulator >= 1.0f)
      {
        game -> tick (sim_time, tick_interval, *queue, ui_events, ui_event_count);
        ui_event_count = 0;

        for (auto ent = entities.begin (); ent != entities.end (); ent++)
          (*ent) -> tick (sim_time, tick_interval, *queue, keyboard, mouse_delta);

        sim_time += tick_interval;
        time_accumulator -= 1.0f;
        mouse_motion -= mouse_delta;
      }

      Frame& frame = *renderer;

      frame.width  = width;
      frame.height = height;

      float alpha = time_accumulator;

      game -> render (frame, alpha);

      for (auto ent = entities.begin (); ent != entities.end (); ent++)
        (*ent) -> render (frame, alpha);

      renderer -> render_frame ();

      float real_time = get_time ();
      float delta_real_time = real_time - prev_real_time;
      prev_real_time = real_time;

      if (delta_real_time > 0.25f)
        delta_real_time = 0.25f;
      time_accumulator += delta_real_time / tick_interval;

      return true;
    }

    void EngineImpl::stop ()
    {
      log () << "* Engine stopping\n"
             << "i Simulation time " << sim_time << "s\n";
      running = false;
    }

    void EngineImpl::enable_ui (bool enable)
    {
      host -> enable_ui (enable);
    }

    Entity::Ptr EngineImpl::create_entity (Rk::StringRef class_name, const PropMap* props)
    {
      if (!class_name)
        throw std::invalid_argument ("class_name is nil");

      auto ent = game -> create_entity (class_name, *queue, renderer -> context (), props);

      if (ent)
      {
        entities.push_back (ent);
        return std::move (ent);
      }
      
      throw std::runtime_error ("No such entity class");
    }

    std::shared_ptr <void> EngineImpl::get_object (Rk::StringRef type)
    {
      return host -> get_object (type);
    }

  } // namespace

} // namespace Co
