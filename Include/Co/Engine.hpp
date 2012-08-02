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
    v2f    mouse_accum,
           mouse_delta;

  public:
    Engine (Clock& clock, float tick_frequency) :
      clock      (clock),
      time_delta (1.0f / tick_frequency)
    {
      restart ();
    }
    
    void restart ()
    {
      sim_time    = 0.0f;
      time_accum  = 0.0f;
      mouse_accum = v2f (0.0f, 0.0f);

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

    v2f mouse_step () const
    {
      return mouse_delta;
    }

    float alpha () const
    {
      return time_accum / time_delta;
    }

    void update (v2f mouse_in)
    {
      float real_time = clock.time ();
      float delta_real_time = real_time - prev_real_time;
      prev_real_time = real_time;

      delta_real_time = Rk::clamp (delta_real_time, 0.0f, 0.25f);
      time_accum += delta_real_time;

      mouse_accum += mouse_in;
      mouse_delta = mouse_accum / time_accum;
    }

    bool begin_tick () const
    {
      return time_accum >= time_delta;
    }

    void end_tick ()
    {
      time_accum  -= time_delta;
      mouse_accum -= mouse_delta;
      sim_time    += time_delta;
    }

  };

}

#endif
