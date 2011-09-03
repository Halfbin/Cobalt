//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLBuffer.hpp"

// Uses
#include "GL.hpp"

namespace Co
{
  void GLBuffer::load_data (const void* data, uptr size)
  {
    if (data)
      Rk::require (size != 0, "data with zero size");

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    if (size)
    {
      glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
      check_gl ("glBufferData");
    }

    glBindBuffer (GL_ARRAY_BUFFER, 0);
  }

  GLBuffer::~GLBuffer ()
  {
    glDeleteBuffers (1, &name);
  }

  void GLBuffer::destroy ()
  {
    delete this;
  }

  GLBuffer::GLBuffer (uptr size, const void* data)
  {
    glGenBuffers (1, &name);
    check_gl ("glGenBuffers");

    load_data (data, size);
  }

  void GLBuffer::bind (uint target)
  {
    glBindBuffer (target, name);
    check_gl ("glBindBuffer");
  }

} // namespace Co
