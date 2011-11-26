//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include "RectProgram.hpp"

namespace Co
{
  RectProgram::RectProgram () :
    vertex_shader   ("../Common/Shaders/Rect-Vertex.glsl",   GL_VERTEX_SHADER),
    fragment_shader ("../Common/Shaders/Rect-Fragment.glsl", GL_FRAGMENT_SHADER)
  {
    program.add_shader (vertex_shader);
    program.add_shader (fragment_shader);

    program.fix_attrib ("attrib_rect", attrib_rect);
    
    program.link ();

    ui_to_clip = program.link_uniform ("ui_to_clip");
  }
  
  void RectProgram::use ()
  {
    program.use ();
    glEnableVertexAttribArray (attrib_rect);
  }

  void RectProgram::done ()
  {
    glDisableVertexAttribArray (attrib_rect);
    program.done ();
  }

}
