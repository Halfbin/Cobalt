//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_GAME
#define CO_H_GAME

// Uses
#include <Co/AudioFrame.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/PropMap.hpp>
#include <Co/UIInput.hpp>
#include <Co/Frame.hpp>
#include <Co/Log.hpp>

#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  
  class GameClient
  {
  public:
    typedef std::shared_ptr <GameClient> Ptr;

    virtual void client_start () = 0;
    virtual void client_stop  () = 0;
    virtual void client_input (const UIEvent* ui_events, uptr ui_event_count, const KeyState* kb, v2f mouse_delta) = 0;
    virtual void client_tick  (float time, float step) = 0;
    virtual void render       (Frame& frame, float alpha) = 0;
    virtual void render_audio (AudioFrame& frame) = 0;

  };

  class GameServer
  {
  public:
    typedef std::shared_ptr <GameServer> Ptr;

    virtual void server_start () = 0;
    virtual void server_stop  () = 0;
    virtual void server_tick  (float time, float step) = 0;

  };

  /*class GameFactory
  {
  public:
    typedef std::shared_ptr <GameFactory> Ptr;

    virtual GameClient::Ptr create_client () = 0;
    virtual GameServer::Ptr create_server () = 0;

  };*/

  /*template <typename Client, typename Server>
  class GameFactoryImpl :
    public GameFactory
  {
    virtual GameClient::Ptr create_client ()
    {
      return std::make_shared <Client> ();
    }

    virtual GameServer::Ptr create_server ()
    {
      return std::make_shared <Server> ();
    }

    GameFactoryImpl (void*) { }

  };

  #define CO_GAME_MODULE(Cli, Srv) \
    RK_MODULE_FACTORY (Co::GameFactoryImpl <Cli, Srv>)*/

} // namespace Co

#endif
