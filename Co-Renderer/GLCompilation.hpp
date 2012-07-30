//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLCOMPILATION
#define CO_GLRENDERER_H_GLCOMPILATION

// Implements
#include <Co/GeomCompilation.hpp>

// Uses
#include <Co/RenderContext.hpp>
#include <Co/WorkQueue.hpp>

#include "GLBuffer.hpp"
#include "GL.hpp"

#include <vector>

namespace Co
{
  class GLCompilation :
    public GeomCompilation
  {
    u32                      vao;
    std::vector <GeomAttrib> attribs;
    uptr                     element_size;
    GLBuffer::Ptr            elements;
    GLBuffer::Ptr            indices;
    IndexType                index_type;

		bool first_use ();

    GLCompilation (
      const GeomAttrib* new_attribs,
      const GeomAttrib* new_attribs_end,
      GeomBuffer::Ptr   new_elements,
      GeomBuffer::Ptr   new_indices,
      IndexType         new_index_type
    );

  public:
    typedef std::shared_ptr <GLCompilation> Ptr;

    static Ptr create (
      WorkQueue&        work_queue,
      const GeomAttrib* new_attribs,
      const GeomAttrib* new_attribs_end,
      GeomBuffer::Ptr   new_elements,
      GeomBuffer::Ptr   new_indices,
      IndexType         new_index_type
    );

    ~GLCompilation ();
    
    IndexType get_index_type () const
    {
      return index_type;
    }

    bool use ()
		{
			if (vao)
      {
				glBindVertexArray (vao);
        check_gl ("glBindVertexArray");
        return true;
      }
			else
      {
				return first_use ();
      }
		}
		
    static void done ()
    {
      glBindVertexArray (0);
      check_gl ("glBindVertexArray");
    }

    uptr index_base ()
    {
      return indices -> get_draw_offset ();
    }

    uptr element_base ()
    {
      return elements -> get_draw_offset () / element_size;
    }

  }; // class GLCompilation

} // namespace Co

#endif
