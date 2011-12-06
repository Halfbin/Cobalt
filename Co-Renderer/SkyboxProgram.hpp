//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_SKYBOXPROGRAM
#define CO_GLRENDERER_H_SKYBOXPROGRAM

#include <Rk/Matrix.hpp>

#include "GLShader.hpp"
#include "GL.hpp"

namespace Co
{
  class SkyboxProgram
  {
    GLShader  vertex_shader,
              fragment_shader;
    GLProgram program;

    u32 world_to_clip;

    u32 vertex_buf,
        index_buf,
        vao;

  public:
    enum
    {
      texunit_cube = 0
    };

    SkyboxProgram ();
    ~SkyboxProgram ();
    
    void render (Rk::Matrix4f xform);

  };

}

#endif
