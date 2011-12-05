//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXRENDERER
#define CO_H_IXRENDERER

#include <Rk/StreamForward.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class Clock;
  class IxFrame;
  
  class IxRenderer
  {
  public:
    static const u64 id = 0x98c54ca97b88cd35ull;

    virtual bool init (void* target, Clock* clock, Rk::IxLockedOutStreamImpl* log) = 0;
    
    virtual void start () = 0;
    virtual void stop  () = 0;
    
    virtual IxFrame* begin_frame (
      float prev_time,
      float current_time,
      u32&  old_frame_id,
      u32&  new_frame_id
    ) = 0;
    
    virtual void submit_frame (IxFrame* frame) = 0;

  }; // class IxRenderer

} // namespace Co

#endif
