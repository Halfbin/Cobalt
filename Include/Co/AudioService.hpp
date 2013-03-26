//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOSERVICE
#define CO_H_AUDIOSERVICE

#include <Co/AudioBuffer.hpp>

#include <Co/WorkQueue.hpp>
#include <Co/Log.hpp>

#include <memory>

namespace Co
{
  class AudioService
  {
  public:
    typedef std::shared_ptr <AudioService> Ptr;

    virtual AudioBuffer::Ptr create_buffer (
      AudioChannels channels,
      u32 samples_per_sec,
      AudioFormat format,
      u32 samples,
      const void* data
    ) = 0;

  };

  class AudioRoot
  {
  public:
    virtual AudioService::Ptr create_audio_service (Log& log, WorkQueue& queue) = 0;

  };

}

#endif
