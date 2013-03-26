//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/AudioService.hpp>

// Uses
#include <Rk/Modular.hpp>

#include "XA2Buffer.hpp"

namespace Co
{
  //
  // = XA2Service ======================================================================================================
  //
  class XA2Service :
    public AudioService
  {
    virtual AudioBuffer::Ptr create_buffer (
      AudioChannels channels,
      u32 samples_per_sec,
      AudioFormat format,
      u32 samples,
      const void* data
    );

  public:
    XA2Service (Log& log, WorkQueue& queue);

  };

  AudioBuffer::Ptr XA2Service::create_buffer (
    AudioChannels channels,
    u32 samples_per_sec,
    AudioFormat format,
    u32 samples,
    const void* data)
  {
    return std::make_shared <XA2Buffer> (channels, samples_per_sec, format, samples, data);
  }

  XA2Service::XA2Service (Log& log, WorkQueue& queue)
  {

  }

  //
  // = Module ==========================================================================================================
  //
  class Root :
    public AudioRoot
  {
    virtual AudioService::Ptr create_audio_service (Log& log, WorkQueue& queue)
    {
      return std::make_shared <XA2Service> (log, queue);
    }

  };

  RK_MODULE (Root);

} // namespace Co
