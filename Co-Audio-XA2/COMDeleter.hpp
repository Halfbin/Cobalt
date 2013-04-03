//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_AUDIO_XA2_H_COMDELETER
#define CO_AUDIO_XA2_H_COMDELETER

#include <XAudio2.h>

namespace Co
{
  struct UnknownDeleter
  {
    void operator () (IUnknown* ix) const
    {
      ix -> Release ();
    }

  };

  struct VoiceDeleter
  {
    void operator () (IXAudio2Voice* voice) const
    {
      voice -> DestroyVoice ();
    }

  };

}

#endif
