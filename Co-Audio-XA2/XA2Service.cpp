//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/AudioService.hpp>

// Uses
#include <Rk/Modular.hpp>
#include <Rk/Mutex.hpp>

#include "COMDeleter.hpp"
#include "XA2Buffer.hpp"

#include <XAudio2.h>

#include <vector>
#include <deque>

namespace Co
{
  WAVEFORMATEX to_wfex (const AudioFormat& fmt)
  {
    if (fmt.coding != audio_coding_pcm)
      throw std::logic_error ("XA2Source: Only PCM coding currently supported");

    if (fmt.channels == audio_channels_invalid)
      throw std::logic_error ("XA2Source: Invalid channel configuration specified");

    WAVEFORMATEX wfex = { 0 };
    wfex.wFormatTag = WAVE_FORMAT_PCM;
    wfex.nChannels  = fmt.channels == audio_channels_stereo ? 2 : 1;
    wfex.nSamplesPerSec = fmt.samples_per_second;
    wfex.wBitsPerSample = fmt.bits_per_sample;
    wfex.cbSize = 0;

    // Correct only for PCM
    wfex.nBlockAlign = (wfex.wBitsPerSample / 8) * wfex.nChannels;
    wfex.nAvgBytesPerSec = wfex.nBlockAlign * wfex.nSamplesPerSec;

    return wfex;
  }

  class SourcePool;

  class SourceVoice :
    private IXAudio2VoiceCallback
  {
    virtual void OnVoiceProcessingPassStart (u32 required) { }
    virtual void OnVoiceProcessingPassEnd   () { }
    virtual void OnBufferStart              (void* ctx) { }
    virtual void OnBufferEnd                (void* ctx) { }
    virtual void OnLoopEnd                  (void* ctx) { }
    virtual void OnVoiceError               (void* ctx, HRESULT code) { }

    virtual void OnStreamEnd ();
    
    SourcePool&           pool;
    std::shared_ptr
    <IXAudio2SourceVoice> ptr;

  public:
    typedef std::shared_ptr <SourceVoice> Ptr;

    SourceVoice (SourcePool& parent, IXAudio2& ixa2, const WAVEFORMATEX& wfex) :
      pool (parent)
    {
      IXAudio2SourceVoice* pisv;
      auto hr = ixa2.CreateSourceVoice (&pisv, &wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
      if (FAILED (hr))
        Rk::raise () << "XA2Service: IXAudio2::CreateSourceVoice failed; HRESULT " << Rk::in_hex (ulong (hr));
      ptr.reset (pisv, VoiceDeleter ());
      OnStreamEnd (); // to add to pool
    }

  };

  class SourcePool
  {
    std::vector
    <SourceVoice::Ptr>     voices;
    std::deque
    <IXAudio2SourceVoice*> free_voices;
    Rk::Mutex              free_voices_mutex;

    friend class SourceVoice;

    void add_free_voice (IXAudio2SourceVoice* isv)
    {
      auto lock = free_voices_mutex.get_lock ();
      free_voices.push_back (isv);
    }

  public:
    void populate (IXAudio2& ixa2, const WAVEFORMATEX& wfex, uint size)
    {
      voices.resize (size);

      for (auto v = voices.begin (), e = voices.end (); v != e; v++)
        *v = std::make_shared <SourceVoice> (*this, ixa2, wfex);
    }
    
    IXAudio2SourceVoice* get_free_voice ()
    {
      auto lock = free_voices_mutex.get_lock ();

      IXAudio2SourceVoice* isv = nullptr;
      if (!free_voices.empty ())
      {
        isv = free_voices.front ();
        free_voices.pop_front ();
      }

      return isv;
    }

  };

  void SourceVoice::OnStreamEnd ()
  {
    pool.add_free_voice (ptr.get ());
  }

  //
  // = XA2Service ======================================================================================================
  //
  class XA2Service :
    public AudioService
  {
    Log&       log;
    WorkQueue& queue;

    std::shared_ptr <IXAudio2>               ixa2;
    std::shared_ptr <IXAudio2MasteringVoice> imastering;

    SourcePool source_pool;

    virtual AudioBuffer::Ptr create_buffer (
      AudioFormat format,
      const void* data,
      u32         size,
      u32         samples
    );

    virtual void play_sound (
      AudioBuffer::Ptr buffer
    );


  public:
    XA2Service (Log& log, WorkQueue& queue);
    ~XA2Service ();

  };

  //
  // create_buffer
  //
  AudioBuffer::Ptr XA2Service::create_buffer (
    AudioFormat format,
    const void* data,
    u32         size,
    u32         samples)
  {
    return std::make_shared <XA2Buffer> (format, data, size, samples);
  }

  //
  // play_sound
  // FIXME: add to list, play in batch later
  //
  void XA2Service::play_sound (AudioBuffer::Ptr buffer)
  {
    if (!buffer)
      return; // robust against async resources

    auto actual = std::static_pointer_cast <XA2Buffer> (buffer);
    
    auto voice = source_pool.get_free_voice ();
    if (!voice)
      return;

    auto hr = voice -> SubmitSourceBuffer (actual -> get ());
    if (FAILED (hr))
      Rk::raise () << "XA2Service: play_sound - IXAudio2::SubmitSourceBuffer failed; HRESULT " << Rk::in_hex (ulong (hr));

    hr = voice -> Start ();
  }

  //
  // Constructor
  //
  XA2Service::XA2Service (Log& log, WorkQueue& queue) :
    log   (log),
    queue (queue)
  {
    auto hr = CoInitializeEx (0, COINIT_MULTITHREADED);
    if (FAILED (hr))
      Rk::raise () << "XA2Service: CoInitializeEx failed; HRESULT " << Rk::in_hex (ulong (hr));

    IXAudio2* pixa2;
    hr = XAudio2Create (&pixa2);
    if (FAILED (hr))
      Rk::raise () << "XA2Service: XAudio2Create failed; HRESULT " << Rk::in_hex (ulong (hr));
    ixa2.reset (pixa2, UnknownDeleter ());

    IXAudio2MasteringVoice* pimastering;
    hr = ixa2 -> CreateMasteringVoice (&pimastering);
    if (FAILED (hr))
      Rk::raise () << "XA2Service: IXAudio2::CreateMasteringVoice failed; HRESULT " << Rk::in_hex (ulong (hr));
    imastering.reset (pimastering, VoiceDeleter ());

    imastering -> SetVolume (0.3f);

    // Create 128 source voices for now
    AudioFormat fmt (audio_coding_pcm, audio_channels_mono, 44100, 16);
    auto wfex = to_wfex (fmt);

    source_pool.populate (*ixa2, wfex, 128);
  }
  
  XA2Service::~XA2Service ()
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
