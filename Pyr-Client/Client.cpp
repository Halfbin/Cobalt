//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Pyr/Client.hpp>

#include <Rk/Modular.hpp>

namespace Pyr
{
  class Client :
    public Co::GameClient
  {
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

    }

    virtual void render (Co::Frame& frame, float alpha)
    {

    }

    virtual void render_audio (Co::AudioFrame& frame)
    {

    }

  };

  class Root :
    public ClientFactory
  {
    virtual Co::GameClient::Ptr create_client ()
    {
      return std::make_shared <Client> ();
    }

  };

  RK_MODULE (Root);

}
