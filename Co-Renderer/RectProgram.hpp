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

    u32 ui_to_clip;

    u32 buffer,
        buffer_size,
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

    void upload_rects (const TexRect* rects, u32 count)
    {
      u32 size = count * sizeof (TexRect);

      if (size > buffer_size)
      {
        buffer_size = size;
        glBufferData (GL_ARRAY_BUFFER, size, rects, GL_STREAM_DRAW);
        check_gl ("glBufferData");
      }
      else
      {
        glBufferSubData (GL_ARRAY_BUFFER, 0, size, rects);
        check_gl ("glBufferSubData");
      }
    }
    
  };

}

#endif
