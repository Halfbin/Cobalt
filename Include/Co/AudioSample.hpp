//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOSAMPLE
#define CO_H_AUDIOSAMPLE

// Uses
#include <Co/AudioBuffer.hpp>
#include <Co/AudioFormat.hpp>
#include <Co/WorkQueue.hpp>

#include <Rk/StringRef.hpp>

#include <memory>

namespace Co
{
  class AudioSample
  {
    AudioBuffer::Ptr buffer;
    AudioFormat      format;

    virtual AudioBuffer::Ptr retrieve (AudioFormat& format) = 0;

  public:
    typedef std::shared_ptr <AudioSample> Ptr;

    bool ready ()
    {
      return get () != nullptr;
    }

    AudioBuffer::Ptr get ()
    {
      if (!buffer)
        buffer = retrieve (format);
      return buffer;
    }

    AudioFormat get_format () const
    {
      return format;
    }

  }; // class AudioSample

  class AudioSampleFactory
  {
  public:
    typedef std::shared_ptr <AudioSampleFactory> Ptr;

    virtual AudioSample::Ptr create (
      Rk::StringRef path
    ) = 0;

  };
  
  class AudioSampleRoot
  {
  public:
    virtual AudioSampleFactory::Ptr create_factory (Log& log, WorkQueue& queue) = 0;

  };

} // namespace Co

#endif
