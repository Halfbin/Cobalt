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
    const GeomAttrib* new_attribs, uint new_attrib_count, IxGeomBuffer* new_elements, IxGeomBuffer* new_indices, IndexType new_index_type
  ) :
    vao        (0),
    attribs    (new_attribs, new_attribs + new_attrib_count),
    elements   (static_cast <GLBuffer*> (new_elements)),
    indices    (static_cast <GLBuffer*> (new_indices)),
    index_type (new_index_type)
  { }
  
  bool GLCompilation::first_use ()
  {
    glGenVertexArrays (1, &vao);
		check_gl ("glGenVertexArrays");

		glBindVertexArray (vao);
    check_gl ("glBindVertexArray");
    
		if (!elements -> bind (GL_ARRAY_BUFFER))
    {
      done ();
      return false;
    }

		if (indices)
    {
      if (!indices -> bind (GL_ELEMENT_ARRAY_BUFFER))
      {
        done ();
        return false;
      }
    }

    for (auto attrib = attribs.begin (); attrib != attribs.end (); attrib++)
		{
      static const uint types [4] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_FLOAT };
      uint type = types [attrib -> type];

      glVertexAttribPointer (
				attrib -> index,
				((attrib -> index == attrib_tcoords) ? 2 : 3),
				type,
				false,
				attrib -> stride,
				(void*) uptr (attrib -> offset)
			);
      check_gl ("glVertexAttribPointer");

      glEnableVertexAttribArray (attrib -> index);
      check_gl ("glEnableVertexAttribArray");
		}

    return true;
  }
	
} // namespace Co
