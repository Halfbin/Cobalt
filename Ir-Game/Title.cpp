//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "State.hpp"

// Uses
#include <Co/Texture.hpp>
#include <Co/Engine.hpp>
#include <Co/Frame.hpp>

#include <Rk/ShortString.hpp>

#include "Common.hpp"

namespace Ir
{
  class TitleState :
    public State
  {
    Co::Font::Ptr    font;
    Co::Texture::Ptr skybox;
    float enter_time;

    virtual void precache (Co::WorkQueue& queue);
    virtual bool ready    ();
    virtual void enter    (float time);
    virtual void leave    ();

    virtual void tick (
      Co::WorkQueue&     queue,
      Co::Frame&         frame,
      float              cur_time,
      float              prev_time,
      const Co::UIEvent* ui_events,
      uint               ui_event_count
    );

  } title;

  State& title_state = title;

  void TitleState::precache (Co::WorkQueue& queue)
  {
    if (font)
      return;

    Co::CodeRange code_ranges [] = {
      { 0x0020, 0x00e7 }
    };

    font = font_factory -> create (queue, "../Common/Fonts/agencyb.ttf", 80, Co::fontsize_points, code_ranges);

    skybox = texture_factory -> create (queue, "title.cotexture", false, true);
  }

  bool TitleState::ready ()
  {
    return font -> ready () && skybox -> ready ();
  }

  void TitleState::enter (float time)
  {
    enter_time = time;
  }

  void TitleState::leave ()
  {
    
  }

  template <typename F>
  F time_fade (F time, F length)
  {
    return Rk::clamp (time / length, 0.0f, 1.0f);
  }

  void TitleState::tick (
    Co::WorkQueue&     queue,
    Co::Frame&         frame,
    float              cur_time,
    float              prev_time,
    const Co::UIEvent* ui_events,
    uint               ui_event_count)
  {
    if (!ready ())
      return;

    Co::Spatial prev, cur;

    prev.position = Co::Vector3 (0.0f, 0.0f, 0.0f);
    cur.position  = Co::Vector3 (0.0f, 0.0f, 0.0f);

    auto sky_tex = skybox -> get ();

    float prev_delta = (prev_time - enter_time),
          cur_delta  = (cur_time  - enter_time);

    const auto tilt = Co::Quaternion (0.7f, Co::Vector3 (1.0f, 0.0f, 0.0f));
    prev.orientation = tilt * Co::Quaternion (prev_delta * 0.01f, Co::Vector3 (0.0f, 0.0f, 1.0f));
    cur.orientation  = tilt * Co::Quaternion (cur_delta  * 0.01f, Co::Vector3 (0.0f, 0.0f, 1.0f));

    frame.set_camera (prev, cur, 90.0f, 90.0f, 0.1f, 1000.0f);

    frame.set_skybox (
      sky_tex,
      Co::Vector3 (0.0f, 0.0f, 0.0f),
      time_fade (prev_delta, 3.0f),
      time_fade (cur_delta,  3.0f)
    );

    float t0 = 1.0f - time_fade (prev_delta - 2.0f, 0.5f);
    float t1 = 1.0f - time_fade (cur_delta  - 2.0f, 0.5f);

    float prev_top = 70.0f + 40.0f * std::pow (t0, 3.0f);
    float cur_top  = 70.0f + 40.0f * std::pow (t1, 3.0f);

    Co::Vector3 glass (0.05f, 0.05f, 0.05f);

    // Title bar
    frame.add_label (
      nullptr,
      &Co::TexRect (0, 0, frame.get_width (), font -> height (), 0, 0, 0, 0), 1,
      Co::Spatial2D (Co::Vector2 (0.0f, prev_top)),
      Co::Spatial2D (Co::Vector2 (0.0f, prev_top)),
      nil, nil,
      Co::Vector4 (glass, 0.9f - t0),
      Co::Vector4 (glass, 0.9f - t1)
    );

    draw_ui_text (
      frame,
      Co::Vector2 (80.0f, prev_top + 10.0f),
      Co::Vector2 (80.0f, cur_top  + 10.0f),
      *font,
      (const char16*) L"Iridium",
      Co::Vector4 (1.0f, 1.0f, 1.0f, 1.0f - t0),
      Co::Vector4 (1.0f, 1.0f, 1.0f, 1.0f - t1)
    );

    for (auto iter = ui_events; iter != ui_events + ui_event_count; iter++)
    {
      if (iter -> type == Co::ui_event_mouse_move)
      {
        frame.add_label (
          nullptr,
          &Co::TexRect (-10, -10, 20, 20, 0, 0, 0, 0), 1,
          Co::Spatial2D (Co::Vector2 (iter -> a, iter -> b), Co::rotate (prev_time * 10.0f)),
          Co::Spatial2D (Co::Vector2 (iter -> a, iter -> b), Co::rotate (cur_time  * 10.0f)),
          nil, nil,
          Co::Vector4 (glass, 0.7f),
          Co::Vector4 (glass, 0.7f)
        );

        break;
      }
    }
  }

}
