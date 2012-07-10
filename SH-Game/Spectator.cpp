//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Entity.hpp>

// Uses
#include <Co/EntityClass.hpp>
#include <Co/Frame.hpp>

namespace SH
{
  class Spectator :
    public Co::Entity
  {
    Co::Spatial cur, next;

    virtual void tick (float time, float step, Co::WorkQueue& queue, const Co::KeyState* kb, v2f mouse_delta)
    {
      using namespace Co::Keys;

      cur = next;

      float x_move = 0.0f,
            y_move = 0.0f,
            z_move = 0.0f;

      if      (kb [key_w].down && !kb [key_s].down) x_move =  1.0f;
      else if (kb [key_s].down && !kb [key_w].down) x_move = -1.0f;

      if      (kb [key_a].down && !kb [key_d].down) y_move =  1.0f;
      else if (kb [key_d].down && !kb [key_a].down) y_move = -1.0f;

      if      (kb [key_spacebar].down     && !kb [key_left_control].down) z_move =  1.0f;
      else if (kb [key_left_control].down && !kb [key_spacebar].down    ) z_move = -1.0f;

      float dist = step * 15.0f;

      next.position += cur.orientation.forward () * x_move * dist
                    +  cur.orientation.left    () * y_move * dist
                    +  v3f (0, 0, 1) * z_move * dist;

      next.orientation =
        Co::Quat (step * 20.0f * mouse_delta.x, v3f (0, 0, -1)) *
        cur.orientation *
        Co::Quat (step * 20.0f * mouse_delta.y, v3f (0, 1, 0));
    }

    virtual void render (Co::Frame& frame, float alpha)
    {
      frame.set_camera (lerp (cur, next, alpha), 90.0f, 0.1f, 1000.0f);
    }

    Spectator ()
    {
      cur.position = v3f (0.0f, 0.0f, 80.0f);
      cur.orientation = Co::Quat (0.785f, v3f (0, 0, 1));
      next = cur;
    }
    
  public:
    static Ptr create (Co::WorkQueue& queue, const Co::PropMap* props)
    {
      return Ptr (
        new Spectator (),
        queue.make_deleter <Spectator> ()
      );
    }

  };

  Co::EntityClass <Spectator> ent_class ("Spectator");
  Co::EntityClassBase& spectator_class = ent_class;

}
