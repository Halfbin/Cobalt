//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Co/Frame.hpp>

#include "RectProgram.hpp"

namespace Co
{
  RectProgram::RectProgram () :
    vertex_shader   ("../Common/Shaders/Rect-Vertex.glsl",   GL_VERTEX_SHADER),
    fragment_shader ("../Common/Shaders/Rect-Fragment.glsl", GL_FRAGMENT_SHADER)
  {
    program.add_shader (vertex_shader);
    program.add_shader (fragment_shader);

    program.fix_attrib ("attrib_rect",    attrib_rect);
    program.fix_attrib ("attrib_tcoords", attrib_tcoords);

    program.link ();

    ui_to_clip    = program.link_uniform ("ui_to_clip");
    ui_to_ui      = program.link_uniform ("ui_to_ui");
    linear_colour = program.link_uniform ("linear_colour");
    const_colour  = program.link_uniform ("const_colour");

    program.use ();

    u32 tex = program.link_uniform ("tex");
    glUniform1i (tex, texunit_tex);

    program.done ();

    // Create VAO
    glGenVertexArrays (1, &vao);
    check_gl ("glGenVertexArrays");

    glBindVertexArray (vao);
    check_gl ("glBindVertexArray");

    // Create Buffer
    glGenBuffers (1, &buffer);
    check_gl ("glGenBuffers");

    glBindBuffer (GL_ARRAY_BUFFER, buffer);
    check_gl ("glBindBuffer");

    glBufferData (GL_ARRAY_BUFFER, 1024 * sizeof (TexRect), 0, GL_STREAM_DRAW);
    check_gl ("glBufferData");

    // Attribute format
    glVertexAttribIPointer (attrib_rect, 4, GL_INT, 32, (void*) uptr (0));
    check_gl ("glVertexAttribIPointer");

    glVertexAttribIPointer (attrib_tcoords, 4, GL_INT, 32, (void*) uptr (16));
    check_gl ("glVertexAttribIPointer");

    // Instancing behviour
    glVertexAttribDivisor (attrib_rect, 1);
    check_gl ("glVertexAttribDivisor");

    glVertexAttribDivisor (attrib_tcoords, 1);
    check_gl ("glVertexAttribDivisor");

    // Enable attributes
    glEnableVertexAttribArray (attrib_rect);
    check_gl ("glEnableVertexAttribArray");

    glEnableVertexAttribArray (attrib_tcoords);
    check_gl ("glEnableVertexAttribArray");

    // Done with VAO
    glBindVertexArray (0);
    check_gl ("glBindVertexArray");

    glBindBuffer (GL_ARRAY_BUFFER, 0);
    check_gl ("glBindBuffer");
  }
  
  RectProgram::~RectProgram ()
  {
    glDeleteVertexArrays (1, &vao);
    vao = 0;

    glDeleteBuffers (1, &buffer);
    buffer = 0;
  }

  void RectProgram::use ()
  {
    program.use ();

    glBindVertexArray (vao);
    check_gl ("glBindVertexArray");

    glBindBuffer (GL_ARRAY_BUFFER, buffer);
    check_gl ("glBindBuffer");
  }

  void RectProgram::done ()
  {
    glBindVertexArray (0);
    check_gl ("glBindVertexArray");

    glBindBuffer (GL_ARRAY_BUFFER, 0);
    check_gl ("glBindBuffer");
    
    program.done ();
  }

}
