//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLCompilation.hpp"

// Uses
#include "GLBuffer.hpp"
#include "Common.hpp"

namespace Co
{
  class IxGeomBuffer;

  GLCompilation::~GLCompilation ()
  {
      
  }

  void GLCompilation::destroy ()
  {
      
  }

  GLCompilation::GLCompilation (
    const GeomAttrib* new_attribs, uint new_attrib_count, IxGeomBuffer* new_elements, IxGeomBuffer* new_indices) try
  {
    attribs      = new_attribs;
    attrib_count = new_attrib_count;
    elements     = static_cast <GLBuffer*> (new_elements);
    indices      = static_cast <GLBuffer*> (new_indices);
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLCompilation::GLCompilation");
    throw;
  }

  void GLCompilation::use ()
  {
    if (!vao)
    {
      glGenVertexArrays (1, &vao);
      check_gl ("glGenVertexArrays");

      glBindVertexArray (vao);

      for (uint i = 0; i != attrib_count; i++)
      {
        const GeomAttrib& attrib = attribs [i];
        glVertexAttribPointer (
          attrib.index,
          attrib.index == attrib_tex_coords ? 2 : 3,
          attrib.type,
          false,
          attrib.stride,
          (void*) uptr (attrib.offset)
        );
      }

      elements -> bind (GL_ARRAY_BUFFER);
      if (indices)
        indices  -> bind (GL_ELEMENT_ARRAY_BUFFER);
    }
    else
    {
      glBindVertexArray (vao);
    }
  }

  /*void GLCompilation::done ()
  {
    glBindVertexArray (0);
  }*/

} // namespace Co
