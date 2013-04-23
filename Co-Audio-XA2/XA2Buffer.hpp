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

extern "C"
{
  long _InterlockedIncrement (volatile long*);
  long _InterlockedDecrement (volatile long*);
}

#pragma intrinsic(_InterlockedIncrement, _InterlockedDecrement)

namespace Co
{
  class XA2Buffer :
    public AudioBuffer
  {
    std::vector <u8> store;
    XAUDIO2_BUFFER   xa2buf;
    uint             refs;

    XA2Buffer (
      AudioFormat      format,
      std::vector <u8> data,
      u32              samples
    );

  public:
    typedef std::shared_ptr <XA2Buffer> Ptr;

    void acquire ();
    void release ();

    static Ptr create (
      AudioFormat      format,
      std::vector <u8> data,
      u32              samples
    );

    XAUDIO2_BUFFER* get ();

    u8* data ()
    {
      return store.data ();
    }

  };

}

#endif
