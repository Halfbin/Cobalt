//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_NET_H_PROXY
#define CO_NET_H_PROXY

// Uses
#include <Co/NetChannel.hpp>

namespace Co
{
  class NetProxy
  {
  public:
    virtual void replicate (NetInChannel&) = 0;

  };

} // namespace Co

#endif
