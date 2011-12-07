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
    static const float fade_time;

    Co::IxTexture::Ptr skybox;
    float enter_time;

    virtual void enter (float time);
    virtual void leave ();
    virtual void tick  (Co::IxFrame* frame, float time, float prev_time);

  } title;

  State* title_state = &title;

  const float TitleState::fade_time = 8.0f;

  void TitleState::enter (float time)
  {
    skybox = texture_factory -> create (load_context, "title.cotexture", false, true);
    enter_time = -1.0f;
  }

  void TitleState::leave ()
  {

  }

  void TitleState::tick (Co::IxFrame* frame, float time, float prev_time)
  {
    Co::Spatial prev, cur;

    prev.position = Co::Vector3 (0.0f, 0.0f, 0.0f);
    cur.position  = Co::Vector3 (0.0f, 0.0f, 0.0f);

    auto sky_tex = skybox -> get ();

    if (!sky_tex)
      return;

    if (enter_time < 0.0f)
      enter_time = prev_time + 1.0f;

    const auto tilt = Co::Quaternion (0.7f, Co::Vector3 (1.0f, 0.0f, 0.0f));
    prev.orientation = tilt * Co::Quaternion ((prev_time - enter_time) * 0.01f, Co::Vector3 (0.0f, 0.0f, 1.0f));
    cur.orientation  = tilt * Co::Quaternion ((time      - enter_time) * 0.01f, Co::Vector3 (0.0f, 0.0f, 1.0f));

    frame -> set_camera (prev, cur, 90.0f, 90.0f, 0.1f, 1000.0f);

    float prev_alpha = 1.0f,
          cur_alpha  = 1.0f;

    if (prev_time - enter_time < fade_time)
      prev_alpha = (prev_time - enter_time) / fade_time;

    if (time - enter_time < fade_time)
      cur_alpha = (time - enter_time) / fade_time;

    frame -> set_skybox (sky_tex, Co::Vector3 (0.0f, 0.0f, 0.0f), prev_alpha, cur_alpha);
  }

}
