//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXLOADER
#define CO_H_IXLOADER

#include <Rk/Types.hpp>

namespace Co
{
  class IxRenderDevice;

  class IxLoader
  {
  public:
    enum : u64 { id = 0x93e68db4d3a71fdd  };

    virtual void init (IxRenderDevice* device) = 0;

    virtual void start () = 0;
    virtual void stop  () = 0;

    virtual void cancel_all () = 0;
    virtual void collect () = 0;

  };

}

#endif
