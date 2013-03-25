//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef PYR_SERVER_H_SERVER
#define PYR_SERVER_H_SERVER

#include <Co/Game.hpp>

namespace Pyr
{
  class ServerFactory
  {
  public:
    virtual Co::GameServer::Ptr create_server () = 0;

  };

}

#endif
