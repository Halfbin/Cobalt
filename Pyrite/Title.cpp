//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Co/Game.hpp>
#include <Co/ClientFrontend.hpp>
#include <Co/Font.hpp>

namespace Pyr
{
  class Title :
    public Co::GameClient
  {
    Co::ClientFrontend& frontend;
    Co::Font::Ptr font;

    virtual void client_start ()
    {

    }

    virtual void client_stop ()
    {

    }

    virtual void client_input (const Co::UIEvent* ui_events, uptr ui_event_count, const Co::KeyState* kb, v2f mouse_delta)
    {

    }

    virtual void client_tick (float time, float step)
    {
      /*if (false)
      {
        frontend.begin_game (
          frontend.load_client ("Pyr-Client"),
          frontend.load_server ("Pyr-Server")
        );
      }*/

    }

    virtual void render (Co::Frame& frame, float alpha)
    {

    }

  public:
    Title (Co::ClientFrontend& frontend) :
      frontend (frontend)
    {
      auto ff = frontend.load_module <Co::FontFactory> ("Co-Font", 0);
      font = ff -> create ("common/fonts/palab.ttf", 32, Co::fontsize_points, ff -> get_repetoire ("WGL4"), 0);
    }
    
  };

  Co::GameClient::Ptr create_title (Co::ClientFrontend& frontend)
  {
    return std::make_shared <Title> (frontend);
  }

}
