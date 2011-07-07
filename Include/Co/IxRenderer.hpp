//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXRENDERER
#define CO_H_IXRENDERER

#include <Rk/Types.hpp>

namespace Co
{
  class Clock;
  class Frame;

  class IxRenderer
  {
  public:
    enum : u64 { id = 0x98c54ca97b88cd35ull };

    virtual void init (void* target, Clock* clock) = 0;
    
    virtual void start () = 0;
    virtual void stop  () = 0;
    
    virtual Frame* begin_frame (
      float prev_time,
      float current_time,
      u32&  old_frame_id,
      u32&  new_frame_id
    ) = 0;
    
    virtual void submit_frame (Frame* frame) = 0;

  }; // class IxRenderer

} // namespace Co

#endif
