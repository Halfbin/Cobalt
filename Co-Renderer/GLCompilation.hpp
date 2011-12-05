//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLCOMPILATION
#define CO_GLRENDERER_H_GLCOMPILATION

// Implements
#include <Co/IxGeomCompilation.hpp>

// Uses
#include <Co/IxRenderContext.hpp>

#include "GL.hpp"

#include <vector>

namespace Co
{
  class GLBuffer;
  class IxGeomBuffer;

  class GLCompilation :
    public IxGeomCompilation
  {
    u32                      vao;
    std::vector <GeomAttrib> attribs;
    GLBuffer*                elements;
    GLBuffer*                indices;
    IndexType                index_type;

    ~GLCompilation ();
    virtual void destroy ();

		bool first_use ();

  public:
    GLCompilation (
      const GeomAttrib* new_attribs,
      const GeomAttrib* new_attribs_end,
      IxGeomBuffer*     new_elements,
      IxGeomBuffer*     new_indices,
      IndexType         new_index_type
    );

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

  }; // class GLCompilation

} // namespace Co

#endif
