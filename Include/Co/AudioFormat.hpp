//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOFORMAT
#define CO_H_AUDIOFORMAT

#include <Rk/Types.hpp>

namespace Co
{
  enum AudioChannels
  {
    audio_channels_invalid = 0,

    audio_channels_mono,
    audio_channels_stereo
  };

  enum AudioCoding
  {
    audio_coding_invalid = 0,

    audio_coding_pcm

  };

  struct AudioFormat
  {
    AudioCoding   coding;
    AudioChannels channels;
    u32           samples_per_second,
                  bits_per_sample;

    AudioFormat () :
      coding             (audio_coding_invalid),
      channels           (audio_channels_invalid),
      samples_per_second (0),
      bits_per_sample    (0)
    { }
    
    AudioFormat (AudioCoding code, AudioChannels chans, u32 sps, u32 bps) :
      coding             (code),
      channels           (chans),
      samples_per_second (sps),
      bits_per_sample    (bps)
    { }
    
  };

}

#endif
