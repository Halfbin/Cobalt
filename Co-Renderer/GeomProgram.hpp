//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GEOMPROGRAM
#define CO_GLRENDERER_H_GEOMPROGRAM

#include <Co/IxGeomCompilation.hpp>

#include <Rk/Matrix.hpp>

#include "GLShader.hpp"
#include "GL.hpp"

namespace Co
{
  class GeomProgram
  {
    GLShader  vertex_shader,
              fragment_shader;
    GLProgram program;

    u32 world_to_clip,
        world_to_eye,
        model_to_world;

  public:
    enum
    {
      attrib_position = Co::attrib_position,
      attrib_tcoords  = Co::attrib_tcoords,
      attrib_normal   = Co::attrib_normal
    };

    GeomProgram ();
    
    void use  ();
    void done ();

    void set_world_to_clip (Rk::Matrix4f xform)
    {
      glUniformMatrix4fv (world_to_clip, 1, true, xform.raw ());
      check_gl ("glUniformMatrix4fv");
    }

    void set_world_to_eye (Rk::Matrix4f xform)
    {
      glUniformMatrix4fv (world_to_eye, 1, true, xform.raw ());
      check_gl ("glUniformMatrix4fv");
    }

    void set_model_to_world (Rk::Matrix4f xform)
    {
      glUniformMatrix4fv (model_to_world, 1, true, xform.raw ());
      check_gl ("glUniformMatrix4fv");
    }

  };

}

#endif
