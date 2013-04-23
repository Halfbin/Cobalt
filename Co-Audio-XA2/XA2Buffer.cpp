//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "XA2Buffer.hpp"

namespace Co
{
  XA2Buffer::XA2Buffer (
    AudioFormat      format,
    std::vector <u8> data,
    u32              samples
  ) :
    store (std::move (data)),
    refs  (1)
  {
    xa2buf.Flags = XAUDIO2_END_OF_STREAM;
    xa2buf.AudioBytes = (u32) store.size ();
    xa2buf.pAudioData = store.data ();
    xa2buf.PlayBegin  = 0;
    xa2buf.PlayLength = samples;
    xa2buf.LoopBegin  = 0;
    xa2buf.LoopLength = 0;
    xa2buf.LoopCount  = 0;
    xa2buf.pContext   = this;
  }

  XA2Buffer::Ptr XA2Buffer::create (
    AudioFormat      format,
    std::vector <u8> data,
    u32              samples)
  {
    return Ptr (
      new XA2Buffer (format, std::move (data), samples),
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
