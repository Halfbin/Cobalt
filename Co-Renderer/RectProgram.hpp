//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_RECTPROGRAM
#define CO_GLRENDERER_H_RECTPROGRAM

#include <Rk/Matrix.hpp>

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
        tex_to_colour;

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

    void set_ui_to_clip (Rk::Matrix2x3f xform)
    {
      glUniformMatrix3x2fv (ui_to_clip, 1, true, xform.raw ());
      check_gl ("glUniformMatrix3x2fv");
    }

    void set_tex_to_colour (Rk::Matrix4f xform)
    {
      glUniformMatrix4fv (tex_to_colour, 1, true, xform.raw ());
      check_gl ("glUniformMatrix4fv");
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