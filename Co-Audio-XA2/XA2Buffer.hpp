//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_AUDIO_XA2_H_XA2BUFFER
#define CO_AUDIO_XA2_H_XA2BUFFER

// Implements
#include <Co/AudioBuffer.hpp>

// Uses
#include <Co/AudioFormat.hpp>

#include <Rk/Types.hpp>

#include <XAudio2.h>

#include <vector>

namespace Co
{
  class XA2Buffer :
    public AudioBuffer
  {
    std::vector <u8> store;
    XAUDIO2_BUFFER   xa2buf;

  public:
    XA2Buffer (
      AudioFormat format,
      const void* data,
      u32         size,
      u32         samples
    );
    
    XAUDIO2_BUFFER* get ();

  };

}

#endif
