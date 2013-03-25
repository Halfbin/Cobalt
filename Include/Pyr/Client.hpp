//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef PYR_CLIENT_H_CLIENT
#define PYR_CLIENT_H_CLIENT

#include <Co/Game.hpp>

namespace Pyr
{
  class ClientFactory
  {
  public:
    virtual Co::GameClient::Ptr create_client () = 0;

  };

}

#endif
