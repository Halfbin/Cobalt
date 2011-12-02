//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxEntity.hpp>

// Uses
#include <Co/EntityClass.hpp>
#include <Co/Frame.hpp>

namespace
{
  class Spectator :
    public Co::IxEntity
  {
    Co::Spatial spatial;

    virtual void destroy ()
    {
      delete this;
    }

    virtual void tick (float time, float prev_time, Co::Frame& frame)
    {
      Co::Spatial new_spatial = spatial;
      //new_spatial.orientation *= Co::Quaternion (0.2f * (time - prev_time), Co::Vector3 (0.0f, 0.0f, 1.0f));
      //new_spatial.orientation.normalize ();
      new_spatial.position -= Co::Vector3 ((time - prev_time), 0.0f, 0.0f);

      frame.set_camera (spatial, new_spatial, 75.0f, 75.0f, 0.1f, 1000.0f);

      spatial = new_spatial;
    }

  public:
    Spectator (Co::IxLoadContext& load_context, Co::IxPropMap* props)
    {
      spatial.position = Co::Vector3 (64.5f, 64.5f, 64.5f);
    }

  };

  Co::EntityClass <Spectator> ent_class ("Spectator");
}

Co::IxEntityClass* spectator_class = &ent_class;
