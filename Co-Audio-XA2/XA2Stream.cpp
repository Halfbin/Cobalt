//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include "XA2Stream.hpp"

namespace Co
{
  XA2Stream::XA2Stream (
    WorkQueue&  queue,
    FileIn::Ptr source
  ) :
    queue  (queue),
    source (source)
  {

  }
  
  void XA2Stream::OnBufferEnd (void* raw)
  {
    auto buf = (XA2Buffer*) raw;

    source -> read (
  }

  void XA2Stream::buffer_filled (WorkQueue& queue, XA2Buffer* buf)
  {
    auto hr = voice -> SubmitSourceBuffer (buf -> get ());
    if (FAILED (hr))
      Rk::raise () << "Audio-XA2: error submitting stream buffer");

  }

  void XA2Stream::start ()
  {

  }

}