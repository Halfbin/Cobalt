//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_NET_H_ENTITY
#define CO_NET_H_ENTITY

#include <Co/Entity.hpp>

// Uses
#include <Co/NetChannel.hpp>

namespace Co
{
  class NetEntity :
    public Entity
  {
  public:
    virtual void replicate (NetOutChannel&) = 0;

  };

} // namespace Co

#endif
