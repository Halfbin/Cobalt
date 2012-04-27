//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_RECTPROGRAM
#define CO_GLRENDERER_H_RECTPROGRAM

#include <Co/Spatial.hpp>

#include "GLShader.hpp"
#include "GL.hpp"

namespace Co
{
  class RectProgram
  {
    GLShader  vertex_shader,
              fragment_shader;
    GLProgram program;

    u32 ui_to_clip,
        ui_to_ui,
        linear_colour,
        const_colour;

    u32 buffer,
        vao;

  public:
    enum
    {
      attrib_rect    = 0,
      attrib_tcoords = 1,

      texunit_tex = 0
    };

    RectProgram ();
    ~RectProgram ();

    void use  ();
    void done ();

    void set_ui_to_clip (Rk::Matrix3f xform)
    {
      glUniformMatrix3fv (ui_to_clip, 1, true, xform.raw ());
      check_gl ("glUniformMatrix3x2fv");
    }

    void set_linear_colour (Vector4 colour)
    {
      glUniform4f (linear_colour, colour.x, colour.y, colour.z, colour.w);
      check_gl ("glUniform4f");
    }

    void set_const_colour (Vector4 colour)
    {
      glUniform4f (const_colour, colour.x, colour.y, colour.z, colour.w);
      check_gl ("glUniform4f");
    }

    void set_transform (Spatial2D xform)
    {
      auto mat = xform.to_matrix ();
      glUniformMatrix3fv (ui_to_ui, 1, true, mat.raw ());
      check_gl ("glUniformMatrix3fv");
    }

    void upload_rects (const TexRect* rects, u32 count)
    {
      if (!count)
        return;

      /*glBufferSubData (GL_ARRAY_BUFFER, 0, count * sizeof (TexRect), rects);
      check_gl ("glBufferData");*/

      for (;;)
      {
        void* mapping = glMapBufferRange (GL_ARRAY_BUFFER, 0, 1024 * sizeof (TexRect), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        check_gl ("glMapBuffer");

        std::copy (rects, rects + count, (TexRect*) mapping);

        auto ok = glUnmapBuffer (GL_ARRAY_BUFFER);
        check_gl ("glUnmapBuffer");

        if (ok)
          break;
      }
    }
    
  };

}

#endif