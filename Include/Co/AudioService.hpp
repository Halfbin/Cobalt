//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOSERVICE
#define CO_H_AUDIOSERVICE

#include <Co/AudioBuffer.hpp>
#include <Co/AudioFormat.hpp>
#include <Co/AudioFrame.hpp>

#include <Co/WorkQueue.hpp>
#include <Co/Log.hpp>

#include <memory>

namespace Co
{
  class AudioService :
    public AudioFrame
  {
  public:
    typedef std::shared_ptr <AudioService> Ptr;

    virtual AudioBuffer::Ptr create_buffer (
      AudioFormat format,
      const void* data,
      u32         size,
      u32         samples
    ) = 0;
    
  };

  class AudioRoot
  {
  public:
    virtual AudioService::Ptr create_audio_service (Log& log, WorkQueue& queue) = 0;

  };

}

#endif
