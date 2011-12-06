//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "State.hpp"

// Uses
#include <Co/IxTexture.hpp>
#include <Co/IxEngine.hpp>
#include <Co/IxFrame.hpp>

#include "Common.hpp"

namespace Ir
{
  class TitleState :
    public State
  {
    Co::IxTexture::Ptr skybox;

    virtual void enter ();
    virtual void leave ();
    virtual void tick  (Co::IxFrame* frame, float time, float prev_time);

  } title;

  State* title_state = &title;

  void TitleState::enter ()
  {
    skybox = texture_factory -> create (load_context, "title.cotexture", false, true);
  }

  void TitleState::leave ()
  {

  }

  void TitleState::tick (Co::IxFrame* frame, float time, float prev_time)
  {
    Co::Spatial prev, cur;

    prev.position = Co::Vector3 (0.0f, 0.0f, 0.0f);
    cur.position  = Co::Vector3 (0.0f, 0.0f, 0.0f);

    const auto tilt = Co::Quaternion (0.7f, Co::Vector3 (1.0f, 0.0f, 0.0f));
    prev.orientation = tilt * Co::Quaternion (prev_time * 0.01f, Co::Vector3 (0.0f, 0.0f, 1.0f));
    cur.orientation  = tilt * Co::Quaternion (time      * 0.01f, Co::Vector3 (0.0f, 0.0f, 1.0f));

    frame -> set_camera (prev, cur, 90.0f, 90.0f, 0.1f, 1000.0f);
    frame -> set_skybox (skybox -> get ());
  }

}
