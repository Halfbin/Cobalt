//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//
/*
#ifndef CO_SERVER_H_NET
#define CO_SERVER_H_NET

#include <Rk/Types.hpp>

namespace Co
{
  class Net
  {
  public:
    virtual void init     (uint concurrency = 0) = 0;
    virtual void shutdown () = 0;

    virtual void open  (u16 port) = 0;
    virtual void close () = 0;

    virtual void work () = 0;

  };

  extern Net& net;

}

#endif
*/