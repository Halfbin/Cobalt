//
// Copyright (C) 2012 Roadkill Software
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
      glBufferData (GL_ARRAY_BUFFER, size, data, stream ? GL_STREAM_DRAW : GL_STATIC_DRAW);
      check_gl ("glBufferData");
    }
  }

  void* GLBuffer::begin (uptr reserve)
  {
    if (!reserve)
      throw std::invalid_argument ("Cannot map zero-length range");

    if (reserve > capacity)
      throw std::invalid_argument ("Cannot map range larger than buffer");

    reserve = Rk::align (reserve, stream_align);

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    u32 inval_flag = GL_MAP_INVALIDATE_RANGE_BIT;

    if (map_offset + reserve > capacity)
    {
      map_offset = 0;
      inval_flag = GL_MAP_INVALIDATE_BUFFER_BIT;
    }

    void* map = glMapBufferRange (
      GL_ARRAY_BUFFER,
      map_offset,
      reserve,
      inval_flag | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_WRITE_BIT
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

    map_length = reserve;
    
    return map;
  }

  bool GLBuffer::commit (uptr length)
  {
    if (map_length == 0)
      throw std::logic_error ("Attempt to commit data to stream before mapping");

    length = Rk::align (length, stream_align);

    glBindBuffer (GL_ARRAY_BUFFER, name);
    check_gl ("glBindBuffer");

    map_length = 0;

    auto ok = glUnmapBuffer (GL_ARRAY_BUFFER);
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

    load_data (data, stream ? capacity : size);
  }

} // namespace Co
