//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_SKYBOXPROGRAM
#define CO_GLRENDERER_H_SKYBOXPROGRAM

#include <Co/Spatial.hpp>
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

    u32 world_to_clip,
        blend_colour;

    u32 vertex_buf,
        index_buf,
        vao;

  public:
    typedef std::shared_ptr <SkyboxProgram> Ptr;
    static inline Ptr create () { return std::make_shared <SkyboxProgram> (); }

    enum
    {
      texunit_cube = 0
    };

    SkyboxProgram ();
    ~SkyboxProgram ();
    
    void render (mat4f xform, v3f colour, float alpha);

  };

  extern SkyboxProgram skybox_program;
}

#endif
