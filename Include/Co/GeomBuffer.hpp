//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_GEOMBUFFER
#define CO_H_GEOMBUFFER

#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  class GeomBuffer
  {
  public:
    typedef std::shared_ptr <GeomBuffer> Ptr;

    virtual void load_data (const void* data, uptr size) = 0;

  };

  class StreamBuffer :
    public GeomBuffer
  {
  public:
    typedef std::shared_ptr <StreamBuffer> Ptr;

    virtual void* begin  (uptr max_size) = 0;
    virtual bool  commit (uptr size) = 0;

  };

}

#endif
