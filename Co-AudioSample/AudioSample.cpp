//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/AudioSample.hpp>

// Uses
#include <Co/ResourceFactory.hpp>
#include <Co/AudioService.hpp>
#include <Co/Filesystem.hpp>
#include <Co/WorkQueue.hpp>

#include <Rk/AsyncMethod.hpp>
#include <Rk/ChunkLoader.hpp>
#include <Rk/ByteStream.hpp>
#include <Rk/Modular.hpp>
#include <Rk/Mutex.hpp>
#include <Rk/File.hpp>

#include <algorithm>
#include <string>

namespace Co
{
  enum WFTag : u16
  {
    wave_format_pcm = 1
  };
  
  struct WaveFormat
  {
    u16 format_tag,
        channel_count;
    u32 samples_per_sec,
        bytes_per_sec;
    u16 bytes_per_vector,
        bits_per_sample;
  };

  static_assert (sizeof (WaveFormat) == 16, "WaveFormat miscompiled");

  //
  // = SampleImpl ======================================================================================================
  //
  class SampleImpl :
    public AudioSample
  {
    std::string      path;
    AudioBuffer::Ptr buffer;
    AudioFormat      format;
    Rk::Mutex        mutex;

    virtual AudioBuffer::Ptr retrieve (AudioFormat& fmt)
    {
      auto lock = mutex.get_lock ();
      fmt = format;
      return std::move (buffer);
    }
    
  public:
    typedef std::shared_ptr <SampleImpl> Ptr;

    SampleImpl (Rk::StringRef new_path) :
      path (new_path.string ())
    { }
    
    void construct (Ptr& self, WorkQueue& queue, LoadContext& ctx)
    {
      auto file = ctx.fs.open_read (path);

      char magics [12];
      Rk::get (*file, magics);
      if (Rk::StringRef (magics, 4) != "RIFF" || Rk::StringRef (magics + 8, 4) != "WAVE")
        throw std::runtime_error ("Source file corrupt or not a WAV");

      WaveFormat wf = { 0, 0, 0, 0, 0, 0 };
      std::vector <u8> sample_data;

      auto loader = Rk::make_chunk_loader (*file);

      while (loader.resume ())
      {
        switch (loader.type)
        {
          case chunk_type ('f', 'm', 't', ' '):
            if (wf.format_tag != 0)
              Rk::raise () << "AudioSample: \"" << path << "\" contains multiple fmt subchunks";
            if (!sample_data.empty ())
              Rk::raise () << "AudioSample: \"" << path << "\" data subchunk before fmt";
            if (loader.size < sizeof (wf))
              Rk::raise () << "AudioSample: \"" << path << "\" has invalid fmt subchunk";

            Rk::get (*file, wf);
            if (wf.format_tag != wave_format_pcm)
              Rk::raise () << "AudioSample: \"" << path << "\" uses an unsupported encoding";
            if (wf.channel_count == 0)
              Rk::raise () << "AudioSample: \"" << path << "\" contains no channels";
            if (wf.channel_count > 2)
              Rk::raise () << "AudioSample: \"" << path << "\" has more channels than supported";
            if (wf.samples_per_sec == 0)
              Rk::raise () << "AudioSample: \"" << path << "\" has invalid sample rate";
            if (wf.bits_per_sample == 0 || (wf.bits_per_sample % 8))
              Rk::raise () << "AudioSample: \"" << path << "\" has invalid sample width";
            if (wf.bytes_per_vector != (wf.bits_per_sample / 8) * wf.channel_count)
              Rk::raise () << "AudioSample: \"" << path << "\" specifies incorrect block alignment";
            if (wf.bytes_per_sec != wf.bytes_per_vector * wf.samples_per_sec)
              Rk::raise () << "AudioSample: \"" << path << "\" specifies incorrect bitrate";

            if (loader.size > sizeof (wf))
              file -> seek (loader.size - sizeof (wf)); // skip any extra header
          break;

          case chunk_type ('d', 'a', 't', 'a'):
            if (!sample_data.empty ())
              Rk::raise () << "AudioSample: \"" << path << "\" contains multiple data subchunks";
            sample_data.resize (loader.size);
            file -> read (sample_data.data (), loader.size);
          break;

          default:
            file -> seek (loader.size);
        }
      }

      if (wf.format_tag == 0)
        Rk::raise () << "AudioSample: \"" << path << "\" contains no fmt subchunk";

      if (sample_data.empty ())
        Rk::raise () << "AudioSample: \"" << path << "\" contains no sample data";

      Co::AudioFormat new_format (
        Co::audio_coding_pcm,
        wf.channel_count == 2 ? Co::audio_channels_stereo : Co::audio_channels_mono,
        wf.samples_per_sec,
        wf.bits_per_sample
      );

      auto new_buffer = ctx.as.create_buffer (
        new_format,
        std::move (sample_data),
        u32 (sample_data.size () / wf.bytes_per_vector)
      );

      auto lock = mutex.get_lock ();
      buffer = std::move (new_buffer);
      format = new_format;
    }

  }; // class SampleImpl

  //
  // = FactoryImpl =====================================================================================================
  //
  class FactoryImpl :
    public AudioSampleFactory,
    public ResourceFactory <std::string, SampleImpl>
  {
    Log&       log;
    WorkQueue& queue;

    virtual AudioSample::Ptr create (Rk::StringRef path)
    {
      auto ptr = find (path.string ());
      if (!ptr)
      {
        ptr = queue.gc_construct (new SampleImpl (path));
        add (path.string (), ptr);
      }
      return std::move (ptr);
    }

  public:
    FactoryImpl (Log& log, WorkQueue& queue) :
      log   (log),
      queue (queue)
    { }
    
  };

  class Root :
    public AudioSampleRoot
  {
    virtual AudioSampleFactory::Ptr create_factory (Log& log, WorkQueue& queue)
    {
      return std::make_shared <FactoryImpl> (log, queue);
    }

  };

  RK_MODULE (Root);

} // namespace Co
