//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLBUFFER
#define CO_GLRENDERER_H_GLBUFFER

// Implements
#include <Co/GeomBuffer.hpp>

// Uses
#include <Co/WorkQueue.hpp>

#include "GL.hpp"

namespace Co
{
  class GLBuffer :
    public GeomBuffer
  {
    u32 name;

    virtual void load_data (const void* data, uptr size);

    GLBuffer (uptr size, const void* data);

  public:
    typedef std::shared_ptr <GLBuffer> Ptr;

    static inline Ptr create (WorkQueue& queue, uptr size, const void* data)
    {
      return Ptr (
        new GLBuffer (size, data),
        queue.make_deleter <GLBuffer> ()
      );
    }

    ~GLBuffer ();
    
    bool bind (uint target)
    {
      if (glIsBuffer (name))
      {
        glBindBuffer (target, name);
        check_gl ("glBindBuffer");
        return true;
      }
      else
      {
        return false;
      }
    }

  }; // class GLBuffer

} // namespace Co

#endif
