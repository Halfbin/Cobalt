//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_AUDIO_XA2_H_XA2STREAM
#define CO_AUDIO_XA2_H_XA2STREAM

// Implements
#include <Co/AudioStream.hpp>

// Uses
#include <Co/Filesystem.hpp>
#include <Co/WorkQueue.hpp>

#include <Rk/Mutex.hpp>
#include <Rk/Types.hpp>

#include <vector>
#include <deque>

#include <XAudio2.h>

#include "XA2Buffer.hpp"

namespace Co
{
  class XA2Stream :
    public AudioStream,
    public IXAudio2VoiceCallback
  {
    WorkQueue&  queue;
    FileIn::Ptr source;

    std::shared_ptr
    <IXAudio2SourceVoice> voice;

    std::vector
    <XA2Buffer::Ptr> buffers;
    std::deque
    <XA2Buffer*>     free_buffers;
    Rk::Mutex        free_buffers_mutex;

    XA2Stream (
      WorkQueue&  queue,
      FileIn::Ptr source
    );

    virtual void start ();
    virtual void stop  ();

    void buffer_filled (WorkQueue&, XA2Buffer*);

    virtual void OnVoiceProcessingPassStart (u32 required) { }
    virtual void OnVoiceProcessingPassEnd   () { }
    virtual void OnBufferStart              (void* ctx) { }
    virtual void OnLoopEnd                  (void* ctx) { }
    virtual void OnVoiceError               (void* ctx, HRESULT code) { }

    virtual void OnBufferEnd (void* ctx);
    virtual void OnStreamEnd ();
  public:
    typedef std::shared_ptr <XA2Stream> Ptr;

    static Ptr create (
      WorkQueue&  queue,
      FileIn::Ptr source
    );

  };

}

#endif
