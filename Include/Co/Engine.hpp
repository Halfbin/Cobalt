//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENGINE
#define CO_H_ENGINE

#include <Co/Clock.hpp>

#include <Rk/Vector2.hpp>
#include <Rk/Clamp.hpp>

namespace Co
{
  class Engine
  {
    Clock& clock;
    float  prev_real_time,
           sim_time,
           time_delta,
           time_accum;
    bool   ticking;

  public:
    Engine (Clock& clock, float tick_frequency) :
      clock      (clock),
      time_delta (1.0f / tick_frequency),
      ticking    (false)
    {
      restart ();
    }
    
    void restart ()
    {
      sim_time    = 0.0f;
      time_accum  = 0.0f;
      
      prev_real_time = clock.time ();
    }

    float time_step () const
    {
      return time_delta;
    }

    float time () const
    {
      return sim_time;
    }

    float alpha () const
    {
      return time_accum / time_delta;
    }

    void update_clock ()
    {
      float real_time = clock.time ();
      float delta_real_time = real_time - prev_real_time;
      prev_real_time = real_time;

      delta_real_time = Rk::clamp (delta_real_time, 0.0f, 0.25f);
      time_accum += delta_real_time;

      ticking = false;
    }

    bool tick ()
    {
      if (ticking)
      {
        time_accum -= time_delta;
        sim_time   += time_delta;
      }

      ticking = time_accum >= time_delta;

      return ticking;
    }

    /*void tick_client (GameClient& gc, Renderer& rr)
    {
      update ();

      while (begin_tick ())
      {
        gc.client_tick (time (), time_step ());
        end_tick ();
      }

      gc.render (rr, alpha ());
    }*/

  };

}

#endif
