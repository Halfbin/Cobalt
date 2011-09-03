//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXRENDERDEVICE
#define CO_H_IXRENDERDEVICE

#include <Rk/Types.hpp>

namespace Co
{
  class IxRenderContext;
  
  class IxRenderDevice
  {
  public:
    static const u64 id = 0x821915e4b144888dull;

    virtual IxRenderContext* create_context () = 0;

    virtual void set_size (uint width, uint height) = 0;

  };

} // namespace Co

#endif
