//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLCOMPILATION
#define CO_GLRENDERER_H_GLCOMPILATION

// Implements
#include <Co/IxGeomCompilation.hpp>

// Uses
#include "GL.hpp"

namespace Co
{
  class GLBuffer;
  class IxGeomBuffer;

  class GLCompilation :
    public IxGeomCompilation
  {
    u32               vao;
    const GeomAttrib* attribs;
    uint              attrib_count;
    GLBuffer*         elements;
    GLBuffer*         indices;

    ~GLCompilation ();
    virtual void destroy ();

		void first_use ();

  public:
    GLCompilation (const GeomAttrib* new_attribs, uint new_attrib_count, IxGeomBuffer* new_elements, IxGeomBuffer* new_indices);

    void use ()
		{
			if (vao)
      {
				glBindVertexArray (vao);
        check_gl ("glBindVertexArray");
      }
			else
      {
				first_use ();
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
