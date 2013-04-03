//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOFRAME
#define CO_H_AUDIOFRAME

#include <Co/AudioBuffer.hpp>

namespace Co
{
  class AudioFrame
  {
  public:
    virtual void play_sound (
      AudioBuffer::Ptr buffer
    ) = 0;

  }; // AudioFrame

} // namespace Co

#endif
