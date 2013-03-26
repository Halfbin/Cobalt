//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOBUFFER
#define CO_H_AUDIOBUFFER

#include <memory>

namespace Co
{
  enum AudioChannels
  {
    audio_mono = 0,
    audio_stereo
  };

  enum AudioFormat
  {
    audio_format_invalid = 0,

    audio_format_pcm_u8,
    audio_format_pcm_i16,
    audio_format_pcm_i24

  };

  class AudioBuffer
  {
  public:
    typedef std::shared_ptr <AudioBuffer> Ptr;

  };

}

#endif
