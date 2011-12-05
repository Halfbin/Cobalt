//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXGAME
#define CO_H_IXGAME

#include <Rk/StreamForward.hpp>
#include <Rk/IxUnique.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class IxEngine;
  class IxUI;

  class IxGame :
    public Rk::IxUnique
  {
  public:
    static const u64 id = 0xb31a8a07f5fb1b71ull;

    typedef Rk::IxUniquePtr <IxGame> Ptr;

    virtual bool init (IxEngine* engine, Rk::IxLockedOutStreamImpl* log) = 0;
    
    virtual void start (IxEngine* engine) = 0;
    virtual void stop  () = 0;
    
    virtual void tick (float time, float prev_time) = 0;
    virtual void update_ui (IxUI* ui) = 0;
    
  };

} // namespace Co

#endif
