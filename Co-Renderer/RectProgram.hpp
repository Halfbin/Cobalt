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

  public:
    enum
    {
      attrib_rect = 0
    };

    RectProgram ();
    
    void use  ();
    void done ();

    void set_ui_to_clip (Rk::Matrix2x3f xform)
    {
      glUniformMatrix4fv (ui_to_clip, 1, true, xform.raw ());
      check_gl ("glUniformMatrix4fv");
    }
    
  };

}

#endif
