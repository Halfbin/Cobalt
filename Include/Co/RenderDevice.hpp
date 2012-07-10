//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_RENDERDEVICE
#define CO_H_RENDERDEVICE

#include <Co/RenderContext.hpp>

#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  class RenderDevice
  {
  public:
    typedef std::shared_ptr <RenderDevice> Ptr;

    virtual RenderContext::Ptr create_context () = 0;

    //virtual void set_size (uint width, uint height) = 0;

  };

} // namespace Co

#endif
