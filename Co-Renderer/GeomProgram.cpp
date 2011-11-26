//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include "GeomProgram.hpp"

namespace Co
{
  GeomProgram::GeomProgram () :
    vertex_shader   ("../Common/Shaders/Geom-Vertex.glsl",   GL_VERTEX_SHADER),
    fragment_shader ("../Common/Shaders/Geom-Fragment.glsl", GL_FRAGMENT_SHADER)
  {
    program.add_shader (vertex_shader);
    program.add_shader (fragment_shader);

    program.fix_attrib ("attrib_position", attrib_position);
    program.fix_attrib ("attrib_normal",   attrib_normal);
    program.fix_attrib ("attrib_tcoords",  attrib_tcoords);

    program.link ();

    world_to_clip  = program.link_uniform ("world_to_clip");
    world_to_eye   = program.link_uniform ("world_to_eye");
    model_to_world = program.link_uniform ("model_to_world");
  }
  
  void GeomProgram::use ()
  {
    program.use ();
    glEnableVertexAttribArray (attrib_position);
    glEnableVertexAttribArray (attrib_normal);
    glEnableVertexAttribArray (attrib_tcoords);
  }

  void GeomProgram::done ()
  {
    glDisableVertexAttribArray (attrib_position);
    glDisableVertexAttribArray (attrib_normal);
    glDisableVertexAttribArray (attrib_tcoords);
    program.done ();
  }

}
