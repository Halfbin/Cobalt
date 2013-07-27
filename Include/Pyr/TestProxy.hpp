//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef PYR_H_PROXY
#define PYR_H_PROXY

// Uses
#include <Co/NetProxy.hpp>
#include <Rk/Vector3.hpp>

namespace Pyr
{
  class TestProxyState
  {
    v3f position;

    template <typename Channel>
    void replicate (Channel& ch)
    {
      ch.position (position);
    }

  };

  class TestProxy :
    public Co::NetProxy
  {
  public:
    virtual void replicate (Co::NetInChannel&);

  };

} // namespace Co

#endif
