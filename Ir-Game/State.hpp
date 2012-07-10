//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef IR_H_STATE
#define IR_H_STATE

#include <Co/WorkQueue.hpp>
#include <Co/UIInput.hpp>
#include <Co/Frame.hpp>

namespace Ir
{
  class State
  {
    static State *current,
                 *next;
    static bool  loading;

    virtual void precache (Co::WorkQueue& queue) = 0;
    virtual bool ready    () = 0;
    virtual void enter    (float time) = 0;
    virtual void leave    () = 0;

    virtual void tick   (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count) = 0;
    virtual void render (Co::Frame& frame, float alpha) = 0;

    static void show_loading_screen (Co::Frame& frame, float time, float step);

  public:
    void switch_to ()
    {
      next = this;
    }

    static void init (State& first)
    {
      current = 0;
      next    = &first;
    }

    static void finish ()
    {
      if (current)
      {
        current -> leave ();
        current = 0;
      }

      next = 0;
    }

    static void tick_current (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count)
    {
      if (next)
      {
        if (current)
          current -> leave ();
        current = next;
        next -> precache (queue);
        loading = true;
        next = 0;
      }

      if (loading)
      {
        loading = !current -> ready ();
        if (!loading)
        {
          current -> enter (time);
        }
        else
        {
          //show_loading_screen (frame, time, step);
          return;
        }
      }

      current -> tick (time, step, queue, ui_events, ui_event_count);
    }

    static void render_current (Co::Frame& frame, float alpha)
    {
      current -> render (frame, alpha);
    }

  };

  extern State&
    title_state;

}

#endif
