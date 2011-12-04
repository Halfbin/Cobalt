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
    //program.fix_attrib ("attrib_normal",   attrib_normal);
    program.fix_attrib ("attrib_tcoords",  attrib_tcoords);
    program.fix_attrib ("attrib_colour",   attrib_colour);

    program.link ();

    program.use ();

    u32 tex_diffuse = program.link_uniform ("tex_diffuse");
    glUniform1i (tex_diffuse, texunit_diffuse);

    glVertexAttrib3f (attrib_colour, 1.0f, 1.0f, 1.0f);
    check_gl ("glVertexAttrib3f");

    program.done ();

    /*world_to_clip  = program.link_uniform ("world_to_clip");
    world_to_eye   = program.link_uniform ("world_to_eye");
    model_to_world = program.link_uniform ("model_to_world");*/
    model_to_clip  = program.link_uniform ("model_to_clip");
  }
  
  void GeomProgram::use ()
  {
    program.use ();
  }

  void GeomProgram::done ()
  {
    program.done ();
  }

}
