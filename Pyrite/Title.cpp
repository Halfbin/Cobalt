//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Pyr/Client.hpp>
#include <Pyr/Server.hpp>

#include <Co/ClientFrontend.hpp>
#include <Co/AudioSample.hpp>
#include <Co/Texture.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>

namespace Pyr
{
  class Title :
    public Co::GameClient
  {
    Co::ClientFrontend& frontend;
    Co::Font::Ptr font;
    Rk::ShortU16String <1024> str;
    Co::Texture::Ptr skybox,
                     boxtex;
    Co::Model::Ptr box;
    Co::AudioSample::Ptr tone;
    
    float t, tp;
    bool begin,
         beep;

    virtual void client_start ()
    {
      frontend.enable_ui (true);
      begin = false;
      beep = false;
    }

    virtual void client_stop ()
    {

    }

    virtual void client_input (const Co::UIEvent* ui_events, uptr ui_event_count, const Co::KeyState* kb, v2f mouse_delta)
    {
      for (auto* e = ui_events; e != ui_events + ui_event_count; e++)
      {
        if (*e == Co::ui_key_up (Co::key_spacebar))
          begin = true;
        else if (*e == Co::ui_key_down (Co::key_b))
          beep = true;
      }
    }

    virtual void client_tick (float time, float step)
    {
      t  = time;
      tp = time + step;

      if (begin)
      {
        auto cfac = frontend.load_module <ClientFactory> ("Pyr-Client");
        auto sfac = frontend.load_module <ServerFactory> ("Pyr-Server");

        frontend.begin_game (
          cfac -> create_client (),
          sfac -> create_server ()
        );
      }

    }

    virtual void render (Co::Frame& frame, float alpha)
    {
      float now = Rk::lerp (t, tp, alpha);

      Co::Quat cam_ori (now * 0.2f, v3f (0, 0, 1));

      frame.set_camera (
        Co::Spatial (
          cam_ori.forward () * -5.0f,
          cam_ori
        ),
        75.0f, 0.1f, 100.0f
      );

      frame.set_skybox (skybox -> get (), nil, 1.0f);

      Co::Material boxmat (nil);
      boxmat.diffuse_tex = boxtex -> get ();

      box -> draw (
        frame,
        Co::Spatial (nil, Co::Quat (now, v3f (1, 0, 0))),
        &boxmat, 1
      );

      Co::draw_ui_text_wrap (
        frame,
        v2f (
          100.0f/* + 50.0f * std::cos (now)*/,
          100.0f/* + 50.0f * std::sin (now)*/
        ),
        frame.width,
        *font,
        Rk::u16_string (L"Press space"),
        Rk::lerp (
          v4f (0, 0, 0, 1),
          v4f (1, 0, 0, 1),
          0.5 + 0.5 * std::sin (now * 3.0f)
        )
      );
    }

    virtual void render_audio (Co::AudioFrame& frame)
    {
      if (beep)
      {
        frame.play_sound (tone -> get ());
        beep = false;
      }
    }

  public:
    Title (Co::ClientFrontend& frontend) :
      frontend (frontend)
    {
      // TODO module configuration; subsystem dependencies and queries
      // auto fontfac = frontend.subsystem <Co::FontFactory> ();
      // font = fontfac -> create (...);

      // Font
      auto ff = frontend.load_module <Co::FontRoot> ("Co-Font") -> create_factory (frontend.get_log (), frontend.get_queue ());
      font = ff -> create ("../common/fonts/palab.ttf", 36, Co::fontsize_points, ff -> get_repetoire ("WGL4"), 0);

      // Textures
      auto tf = frontend.load_module <Co::TextureRoot> ("Co-Texture") -> create_factory (frontend.get_log (), frontend.get_queue ());
      skybox = tf -> create ("../Pyr/title.cotexture");
      boxtex = tf -> create ("../Pyr/derp.cotexture", Co::texture_filter);

      // Model
      auto mf = frontend.load_module <Co::ModelRoot> ("Co-Model") -> create_factory (frontend.get_log (), frontend.get_queue ());
      box = mf -> create ("../Pyr/cube.rkmodel");

      // Sound
      auto asf = frontend.load_module <Co::AudioSampleRoot> ("Co-AudioSample") -> create_factory (frontend.get_log (), frontend.get_queue ());
      tone = asf -> create ("../Pyr/jingle.wav");
    }
    
  };

  extern "C" int __stdcall WinMain (void*, void*, char*, int) try
  {
    Co::ClientFrontend frontend (
      "Pyrite",
      L"Pyrite"
    );

    frontend.run (std::make_shared <Title> (frontend));

    return 0;
  }
  catch (...)
  {
    return 1;
  }

}
