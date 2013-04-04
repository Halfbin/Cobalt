//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "XA2Buffer.hpp"

namespace Co
{
  XA2Buffer::XA2Buffer (
    AudioFormat format,
    const void* data,
    u32         size,
    u32         samples
  ) :
    refs (1)
  {
    auto ptr = (const u8*) data;

    store.assign (ptr, ptr + size);

    xa2buf.Flags = XAUDIO2_END_OF_STREAM;
    xa2buf.AudioBytes = size;
    xa2buf.pAudioData = store.data ();
    xa2buf.PlayBegin  = 0;
    xa2buf.PlayLength = samples;
    xa2buf.LoopBegin  = 0;
    xa2buf.LoopLength = 0;
    xa2buf.LoopCount  = 0;
    xa2buf.pContext   = this;
  }

  XA2Buffer::Ptr XA2Buffer::create (
    AudioFormat format,
    const void* data,
    u32         size,
    u32         samples)
  {
    return Ptr (
      new XA2Buffer (format, data, size, samples),
      [] (XA2Buffer* p) { p -> release (); }
    );
  }

  XAUDIO2_BUFFER* XA2Buffer::get ()
  {
    return &xa2buf;
  }

  void XA2Buffer::acquire ()
  {
    _InterlockedIncrement ((long*) &refs);
  }

  void XA2Buffer::release ()
  {
    if (_InterlockedDecrement ((long*) &refs) == 0)
      delete this;
  }
}
