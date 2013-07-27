//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_NET_H_SERVER
#define CO_NET_H_SERVER

#include <Rk/Types.hpp>

namespace Co
{
  class NetEntity;

  // TODO
  // Time for a proper entity model!

  class NetServer
  {
  public:
    virtual void replicate_new (NetEntity*) = 0;

  };

} // namespace Co

#endif
