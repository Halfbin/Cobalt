//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Pyr/Server.hpp>

#include <Rk/Modular.hpp>

namespace Pyr
{
  class Server :
    public Co::GameServer
  {
    virtual void server_start ()
    {

    }

    virtual void server_stop ()
    {

    }

    virtual void server_tick (float time, float step)
    {

    }

  };

  class Root :
    public ServerFactory
  {
    virtual Co::GameServer::Ptr create_server ()
    {
      return std::make_shared <Server> ();
    }

  };

  RK_MODULE (Root);

}
