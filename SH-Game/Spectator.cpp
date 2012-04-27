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

    virtual void tick (Co::Frame& frame, float time, float prev_time)
    {
      Co::Spatial old_spatial, new_spatial;

      Co::Quaternion a (prev_time * 0.2f, Co::Vector3 (0, 0, 1)),
                     b (time * 0.2f,      Co::Vector3 (0, 0, 1));

      old_spatial.position    = spatial.position + a.forward () * -20.0f;
      old_spatial.orientation = a;
      new_spatial.position    = spatial.position + b.forward () * -20.0f;
      new_spatial.orientation = b;

      //Co::Spatial new_spatial = spatial;
      /*new_spatial.orientation *= Co::Quaternion (0.2f * (time - prev_time), Co::Vector3 (0.0f, 0.0f, 1.0f));
      new_spatial.orientation.normalize ();*/
      //new_spatial.position -= Co::Vector3 (2.0f * (time - prev_time), 0.0f, 0.0f);

      frame.set_camera (old_spatial, new_spatial, 75.0f, 75.0f, 0.1f, 1000.0f);

      //spatial = new_spatial;
    }

    Spectator ()
    {
      spatial.position = Co::Vector3 (96.0f, 96.0f, 126.0f);
      //spatial.orientation = Co::Quaternion (1.57f, Co::Vector3 (0, 0, 1));
    }

  public:
    static Ptr create (Co::WorkQueue& queue, const Co::PropMap& props)
    {
      return Ptr (
        new Spectator (),
        queue.make_deleter <Spectator> ()
      );
    }

  };

  Co::EntityClass <Spectator> ent_class ("Spectator");

}
