//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLBuffer.hpp"

// Uses
#include "Common.hpp"

namespace Co
{
  void GLBuffer::load_data (const void* data, uptr size) try
  {
    if (data && !size)
      throw Rk::Violation ("data with zero size");

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    if (size)
    {
      glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
      check_gl ("glBufferData");
    }

    glBindBuffer (GL_ARRAY_BUFFER, 0);
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLBuffer::load_data")
      << " size: " << size;
    throw;
  }

  GLBuffer::~GLBuffer ()
  {
    glDeleteBuffers (1, &name);
  }

  void GLBuffer::destroy ()
  {
    delete this;
  }

  GLBuffer::GLBuffer (uptr size, const void* data) try
  {
    glGenBuffers (1, &name);
    check_gl ("glGenBuffers");

    load_data (data, size);
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLBuffer::GLBuffer")
      << " size: " << size;
    throw;
  }

  void GLBuffer::bind (uint target)
  {
    glBindBuffer (target, name);
  }

} // namespace Co
