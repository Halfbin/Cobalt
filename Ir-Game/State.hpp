//
// Copyright (C) 2011 Roadkill Software
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

    virtual void tick (
      Co::WorkQueue&     queue,
      Co::Frame&         frame,
      float              cur_time,
      float              prev_time,
      const Co::UIEvent* ui_events,
      uint               ui_event_count
    ) = 0;

    static void show_loading_screen (Co::Frame& frame, float time, float prev_time);

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

    static void tick_current (
      Co::WorkQueue&     queue,
      Co::Frame&         frame,
      float              cur_time,
      float              prev_time,
      const Co::UIEvent* ui_events,
      uint               ui_event_count)
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
          current -> enter (cur_time);
        }
        else
        {
          show_loading_screen (frame, cur_time, prev_time);
          return;
        }
      }

      current -> tick (queue, frame, cur_time, prev_time, ui_events, ui_event_count);
    }

  };

  extern State&
    title_state;

}

#endif
