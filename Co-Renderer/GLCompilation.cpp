//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLCompilation.hpp"

// Uses
#include "GLRenderer.hpp"

namespace Co
{
  GLCompilation::GLCompilation (
    const GeomAttrib* new_attribs,
    const GeomAttrib* new_attribs_end,
    GeomBuffer::Ptr   new_elements,
    GeomBuffer::Ptr   new_indices,
    IndexType         new_index_type
  ) :
    vao        (0),
    attribs    (new_attribs, new_attribs_end),
    elements   (std::static_pointer_cast <GLBuffer> (new_elements)),
    indices    (std::static_pointer_cast <GLBuffer> (new_indices)),
    index_type (new_index_type)
  {
    // Cheating
    element_size = attribs.front ().stride;
  }
  
  GLCompilation::Ptr GLCompilation::create (
    WorkQueue&        work_queue,
    const GeomAttrib* new_attribs,
    const GeomAttrib* new_attribs_end,
    GeomBuffer::Ptr   new_elements,
    GeomBuffer::Ptr   new_indices,
    IndexType         new_index_type)
  {
    return work_queue.gc_attach (
      new GLCompilation (
        new_attribs,
        new_attribs_end,
        new_elements,
        new_indices,
        new_index_type
      )
    );
  }

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
      static const uint types [7] =
      {
        GL_BYTE,
        GL_UNSIGNED_BYTE,
        GL_SHORT,
        GL_INT,
        GL_FLOAT,

        GL_BYTE,
        GL_UNSIGNED_BYTE
      };
      
      uint type   = types [attrib -> type];
      bool normal = attrib -> type > attrib_norm_begin_;

      glVertexAttribPointer (
				attrib -> index,
				((attrib -> index == attrib_tcoords) ? 2 : 3),
				type,
				normal,
				attrib -> stride,
				(void*) uptr (attrib -> offset)
			);
      check_gl ("glVertexAttribPointer");

      glEnableVertexAttribArray (attrib -> index);
      check_gl ("glEnableVertexAttribArray");
		}

    return true;
  }
	
  GLCompilation::~GLCompilation ()
  { }

} // namespace Co
