//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXLOADER
#define CO_H_IXLOADER

#include <Co/IxLoadContext.hpp>

#include <Rk/StreamForward.hpp>
#include <Rk/StringRef.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class IxRenderDevice;
  
  class IxLoader :
    public IxLoadContext
  {
  public:
    static const u64 id = 0x93e68db4d3a71fddull;

    virtual bool init (IxRenderDevice* device, Rk::IxLockedOutStreamImpl* log, Rk::StringRef game_path) = 0;

    virtual void start () = 0;
    virtual void stop  () = 0;

    virtual void cancel_all () = 0;
    virtual void collect () = 0;

  };

} // namespace Co

#endif
