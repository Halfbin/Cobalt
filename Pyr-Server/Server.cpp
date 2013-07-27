//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Pyr/Server.hpp>

// Uses
#include "TestEntity.hpp"

#include <Co/ReplicationServer.hpp>

#include <Rk/Modular.hpp>

#include <memory>
#include <vector>

namespace Pyr
{
  class Server :
    public Co::GameServer
  {
    typedef std::shared_ptr <Co::NetEntity> EntPtr;

    Co::ReplicationServer& repl;

    std::vector <EntPtr> ents;

    void add_ent (Co::NetEntity*&& raw)
    {
      EntPtr ptr (raw);
      repl.entity_new (raw);
      ents.push_back (std::move (ptr));
    }

    virtual void server_start ()
    {
      add_ent (new TestEntity ());
    }

    virtual void server_stop ()
    {

    }

    virtual void server_tick (float time, float step)
    {
      
        (*iter) -> notify ();

      for (auto iter = ents.begin (); iter != ents.end (); iter++)
        (*iter) -> simulate ();
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
