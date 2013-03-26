//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_AUDIO_XA2_H_XA2BUFFER
#define CO_AUDIO_XA2_H_XA2BUFFER

// Implements
#include <Co/AudioBuffer.hpp>

// Uses
#include <Rk/Types.hpp>

namespace Co
{
  class XA2Buffer :
    public AudioBuffer
  {
  public:
    XA2Buffer (
      AudioChannels channels,
      u32 samples_per_sec,
      AudioFormat format,
      u32 samples,
      const void* data
    );
    
  };

}

#endif
