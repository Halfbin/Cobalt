//
// Copyright (C) 2011 Roadkill Software
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
    Co::Spatial spatial;

    virtual void tick (Co::Frame& frame, float time, float prev_time, const Co::KeyState* kb, v2f mouse_delta)
    {
      using namespace Co::Keys;

      Co::Spatial new_spatial = spatial;

      float x_move = 0.0f,
            y_move = 0.0f,
            z_move = 0.0f;

      if      (kb [key_w].down && !kb [key_s].down) x_move =  1.0f;
      else if (kb [key_s].down && !kb [key_w].down) x_move = -1.0f;

      if      (kb [key_a].down && !kb [key_d].down) y_move =  1.0f;
      else if (kb [key_d].down && !kb [key_a].down) y_move = -1.0f;

      if      (kb [key_spacebar].down     && !kb [key_left_control].down) z_move =  1.0f;
      else if (kb [key_left_control].down && !kb [key_spacebar].down    ) z_move = -1.0f;

      float delta = time - prev_time;
      float dist  = delta * 8.0f;

      new_spatial.position += spatial.orientation.forward () * x_move * dist
                           +  spatial.orientation.left    () * y_move * dist
                           +  v3f (0, 0, 1) * z_move * dist;

      /*data.current.orientation =
        Rk::Quaternionf (dx, Rk::Vector3f (0, 0, -1)) *
        data.current.orientation *
        Rk::Quaternionf (dy, Rk::Vector3f (0, 1, 0))
      ;*/

      new_spatial.orientation =
        Co::Quaternion (delta * 10.0f * mouse_delta.x, v3f (0, 0, -1)) *
        spatial.orientation *
        Co::Quaternion (delta * 10.0f * mouse_delta.y, v3f (0, 1, 0));

      //Co::Spatial new_spatial = spatial;
      /*new_spatial.orientation *= Co::Quaternion (0.2f * (time - prev_time), Co::Vector3 (0.0f, 0.0f, 1.0f));
      new_spatial.orientation.normalize ();*/
      //new_spatial.position -= Co::Vector3 (2.0f * (time - prev_time), 0.0f, 0.0f);

      frame.set_camera (spatial, new_spatial, 90.0f, 90.0f, 0.1f, 1000.0f);

      spatial = new_spatial;
    }

    Spectator () :
      spatial (nil)
    {
      spatial.position = v3f (256.0f, 256.0f, 125.0f);
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
