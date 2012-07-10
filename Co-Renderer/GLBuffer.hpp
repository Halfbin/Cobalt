//
// Copyright (C) 2012 Roadkill Software
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
    public StreamBuffer
  {
    u32  name,
         map_offset,
         map_length,
         draw_offset,
         capacity;
    bool stream;

    GLBuffer (bool stream, uptr size, const void* data);

  public:
    typedef std::shared_ptr <GLBuffer> Ptr;

    static inline Ptr create (WorkQueue& queue, bool stream, uptr size, const void* data = 0)
    {
      return Ptr (
        new GLBuffer (stream, size, data),
        queue.make_deleter <GLBuffer> ()
      );
    }

    ~GLBuffer ();

    virtual void load_data (const void* data, uptr size);
    
    virtual void* begin  (uptr max_size);
    virtual bool  commit (uptr size);

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

    u32 get_draw_offset () const
    {
      return draw_offset;
    }

  }; // class GLBuffer

} // namespace Co

#endif
