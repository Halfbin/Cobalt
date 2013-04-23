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

#include <Rk/ByteStream.hpp>

#include <memory>
#include <vector>

namespace Co
{
  class AudioService :
    public AudioFrame
  {
  public:
    typedef std::shared_ptr <AudioService> Ptr;

    virtual AudioBuffer::Ptr create_buffer (
      AudioFormat      format,
      std::vector <u8> data,
      u32              samples
    ) = 0;

    AudioBuffer::Ptr create_buffer (
      AudioFormat format,
      const void* data,
      u32         size,
      u32         samples)
    {
      auto ptr = (const u8*) data;
      return create_buffer (format, std::vector <u8> (ptr, ptr + size), samples);
    }

    virtual void render_frame () = 0;

  };

  class AudioRoot
  {
  public:
    virtual AudioService::Ptr create_audio_service (Log& log, WorkQueue& queue) = 0;

  };

}

#endif
