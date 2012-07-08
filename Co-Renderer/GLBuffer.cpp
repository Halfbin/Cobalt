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
  static const uint stream_align = 64;

  void GLBuffer::load_data (const void* data, uptr size)
  {
    if (data && size == 0)
      return;

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    if (size)
    {
      glBufferData (GL_ARRAY_BUFFER, size, data, stream ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
      check_gl ("glBufferData");
    }

    glBindBuffer (GL_ARRAY_BUFFER, 0);
  }

  void* GLBuffer::begin (uptr max_size)
  {
    if (!max_size)
      throw std::invalid_argument ("Cannot map zero-length range");

    if (max_size > capacity)
      throw std::invalid_argument ("Cannot map range larger than buffer");

    max_size = Rk::align (max_size, stream_align);

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    if (map_offset += max_size > capacity)
    {
      glBufferData (GL_ARRAY_BUFFER, capacity, 0, GL_DYNAMIC_DRAW);
      check_gl ("glBufferData");
      map_offset = 0;
    }

    void* map = glMapBufferRange (
      GL_ARRAY_BUFFER,
      map_offset,
      max_size,
      GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT
    );

    if (!map)
    {
      auto code = glGetError ();
      if (code != GL_OUT_OF_MEMORY)
        throw_gl (code, "glMapBufferRange");
      return nullptr;
    }
    
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    check_gl ("glBindBuffer");

    map_length = max_size;
    
    return map;
  }

  bool GLBuffer::commit (uptr length)
  {
    if (map_length == 0)
      throw std::logic_error ("Attempt to commit data to stream before mapping");

    length = Rk::align (length, stream_align);

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    bool ok = glUnmapBuffer (GL_ARRAY_BUFFER);
    if (!ok)
    {
      auto code = glGetError ();
      if (code != GL_OUT_OF_MEMORY)
        throw_gl (code, "glUnmapBuffer");
    }

    if (!ok)
      return false;

    draw_offset = map_offset;
    map_offset += length;

    return true;
  }

  GLBuffer::~GLBuffer ()
  {
    glDeleteBuffers (1, &name);
  }

  GLBuffer::GLBuffer (bool streaming, uptr size, const void* data)
  {
    if (streaming && size == 0)
      throw std::invalid_argument ("Cannot create zero-length stream buffer");

    stream      = streaming;
    map_offset  = 0;
    map_length  = 0;
    draw_offset = 0;
    capacity    = Rk::align (size, stream_align);

    glGenBuffers (1, &name);
    check_gl ("glGenBuffers");

    load_data (data, streaming ? size : capacity);
  }

} // namespace Co
