//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include "BlockGeomProgram.hpp"

namespace Co
{
  BlockGeomProgram::BlockGeomProgram () :
    vertex_shader   ("../Common/Shaders/BlockGeom-Vertex.glsl",   GL_VERTEX_SHADER),
    fragment_shader ("../Common/Shaders/BlockGeom-Fragment.glsl", GL_FRAGMENT_SHADER)
  {
    program.add_shader (vertex_shader);
    program.add_shader (fragment_shader);

    program.fix_attrib ("attrib_position", attrib_position);
    program.fix_attrib ("attrib_tcoords",  attrib_tcoords);
    program.fix_attrib ("attrib_colour",   attrib_colour);

    program.link ();

    program.use ();

    u32 tex_array = program.link_uniform ("tex_array");
    glUniform1i (tex_array, texunit_array);

    glVertexAttrib4f (attrib_colour, 1.0f, 1.0f, 1.0f, 1.0f);
    check_gl ("glVertexAttrib4f");

    program.done ();

    model_to_clip  = program.link_uniform ("model_to_clip");
  }
  
  void BlockGeomProgram::use ()
  {
    program.use ();
  }

  void BlockGeomProgram::done ()
  {
    program.done ();
  }

}
