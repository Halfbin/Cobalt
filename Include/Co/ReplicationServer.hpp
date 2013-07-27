//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_REPLICATIONSERVER
#define CO_H_REPLICATIONSERVER

#include <Co/NetEntity.hpp>

namespace Co
{
  class ReplicationServer
  {
  public:
    virtual void entity_new       (NetEntity*) = 0;
    virtual void entity_delete    (NetEntity*) = 0;
    virtual void entity_replicate (NetEntity*) = 0;

  };

}

#endif
