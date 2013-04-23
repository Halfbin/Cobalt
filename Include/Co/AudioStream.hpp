//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_AUDIOSTREAM
#define CO_H_AUDIOSTREAM

// Uses
#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  class AudioStream
  {
  public:
    typedef std::shared_ptr <AudioStream> Ptr;

    virtual void start () = 0;
    virtual void stop  () = 0;

  };

}

#endif
