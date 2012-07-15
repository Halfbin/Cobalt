//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_BLOCKGEOMPROGRAM
#define CO_GLRENDERER_H_BLOCKGEOMPROGRAM

#include <Co/GeomCompilation.hpp>

#include <Rk/Matrix.hpp>

#include "GLShader.hpp"
#include "GL.hpp"

namespace Co
{
  class BlockGeomProgram
  {
    GLShader  vertex_shader,
              fragment_shader;
    GLProgram program;

    u32 model_to_clip;

  public:
    typedef std::shared_ptr <BlockGeomProgram> Ptr;
    static inline Ptr create () { return std::make_shared <BlockGeomProgram> (); }

    enum
    {
      attrib_position = Co::attrib_position,
      attrib_tcoords  = Co::attrib_tcoords,
      attrib_colour   = Co::attrib_colour,

      texunit_array = 0
    };

    BlockGeomProgram ();
    
    void use  ();
    void done ();

    void set_model_to_clip (Rk::Matrix4f xform)
    {
      glUniformMatrix4fv (model_to_clip, 1, true, xform.raw ());
      check_gl ("glUniformMatrix4fv");
    }

  };

}

#endif
