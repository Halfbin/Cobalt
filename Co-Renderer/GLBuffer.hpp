//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLBUFFER
#define CO_GLRENDERER_H_GLBUFFER

// Implements
#include <Co/IxGeomBuffer.hpp>

namespace Co
{
  class GLBuffer :
    public IxGeomBuffer
  {
    u32 name;

    virtual void load_data (const void* data, uptr size);

    ~GLBuffer ();
    virtual void destroy ();

  public:
    GLBuffer (uptr size, const void* data);

    void bind (uint target);

  }; // class GLBuffer

} // namespace Co

#endif
