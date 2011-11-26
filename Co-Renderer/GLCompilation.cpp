//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLCompilation.hpp"

// Uses
#include "GLRenderer.hpp"
#include "GLBuffer.hpp"
#include "GL.hpp"

namespace Co
{
  class IxGeomBuffer;

  GLCompilation::~GLCompilation ()
  {
    renderer.add_garbage_vao (vao);
  }

  void GLCompilation::destroy ()
  {
    delete this;
  }

  GLCompilation::GLCompilation (
    const GeomAttrib* new_attribs, uint new_attrib_count, IxGeomBuffer* new_elements, IxGeomBuffer* new_indices)
  {
    vao          = 0;
    attribs      = new_attribs;
    attrib_count = new_attrib_count;
    elements     = static_cast <GLBuffer*> (new_elements);
    indices      = static_cast <GLBuffer*> (new_indices);
  }

  void GLCompilation::first_use ()
  {
    glGenVertexArrays (1, &vao);
		check_gl ("glGenVertexArrays");

		glBindVertexArray (vao);
    check_gl ("glBindVertexArray");
    
		elements -> bind (GL_ARRAY_BUFFER);
		if (indices)
			indices  -> bind (GL_ELEMENT_ARRAY_BUFFER);

		for (uint i = 0; i != attrib_count; i++)
		{
			const GeomAttrib& attrib = attribs [i];

      static const uint types [4] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_FLOAT };
      uint type = types [attrib.type];

      glVertexAttribPointer (
				attrib.index,
				((attrib.index == attrib_tcoords) ? 2 : 3),
				type,
				false,
				attrib.stride,
				(void*) uptr (attrib.offset)
			);
      check_gl ("glVertexAttribPointer");
		}
  }
	
} // namespace Co
