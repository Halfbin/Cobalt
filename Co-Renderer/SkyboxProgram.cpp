//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "SkyboxProgram.hpp"

// Uses
#include <Co/Spatial.hpp>

namespace Co
{
  SkyboxProgram::SkyboxProgram () :
    vertex_shader   ("../Common/Shaders/Skybox-Vertex.glsl",   GL_VERTEX_SHADER),
    fragment_shader ("../Common/Shaders/Skybox-Fragment.glsl", GL_FRAGMENT_SHADER)
  {
    program.add_shader (vertex_shader);
    program.add_shader (fragment_shader);

    program.fix_attrib ("attrib_position", 0);

    program.link ();

    world_to_clip  = program.link_uniform ("world_to_clip");

    program.use ();

    u32 tex_cube = program.link_uniform ("tex_cube");
    glUniform1i (tex_cube, texunit_cube);

    program.done ();

    // Create VAO
    glGenVertexArrays (1, &vao);
    check_gl ("glGenVertexArrays");

    glBindVertexArray (vao);
    check_gl ("glBindVertexArray");

    // Create Buffer
    glGenBuffers (1, &vertex_buf);
    check_gl ("glGenBuffers");

    glBindBuffer (GL_ARRAY_BUFFER, vertex_buf);
    check_gl ("glBindBuffer");

    glGenBuffers (1, &index_buf);
    check_gl ("glGenBuffers");

    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, index_buf);
    check_gl ("glBindBuffer");

    static const Co::Vector3 vertices [6] = {
      Co::Vector3 ( 1,  0,  0), // 0 front
      Co::Vector3 ( 0,  1,  0), // 1 left
      Co::Vector3 (-1,  0,  0), // 2 back
      Co::Vector3 ( 0, -1,  0), // 3 right
      Co::Vector3 ( 0,  0,  1), // 4 top
      Co::Vector3 ( 0,  0, -1), // 5 bottom
    };

    static const u16 indices [24] = {
      0, 4, 1,
      0, 3, 4,
      0, 1, 5,
      0, 5, 3,
      2, 4, 3,
      2, 1, 4,
      2, 3, 5,
      2, 5, 1
    };

    glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
    check_gl ("glBufferData");

    glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indices), indices, GL_STATIC_DRAW);
    check_gl ("glBufferData");

    // Attribute format
    glVertexAttribPointer (0, 3, GL_FLOAT, false, 12, (void*) uptr (0));
    check_gl ("glVertexAttribPointer");

    // Enable attributes
    glEnableVertexAttribArray (0);
    check_gl ("glEnableVertexAttribArray");

    // Done with VAO
    glBindVertexArray (0);
    check_gl ("glBindVertexArray");

    glBindBuffer (GL_ARRAY_BUFFER, 0);
    check_gl ("glBindBuffer");

    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    check_gl ("glBindBuffer");

    glEnable (GL_TEXTURE_CUBE_MAP_SEAMLESS);
  }
  
  SkyboxProgram::~SkyboxProgram ()
  {
    glDeleteVertexArrays (1, &vao);
    vao = 0;

    glDeleteBuffers (1, &vertex_buf);
    vertex_buf = 0;

    glDeleteBuffers (1, &index_buf);
    index_buf = 0;
  }

  void SkyboxProgram::render (Rk::Matrix4f xform)
  {
    program.use ();

    glUniformMatrix4fv (world_to_clip, 1, true, xform.raw ());
    check_gl ("glUniformMatrix4fv");

    glBindVertexArray (vao);
    check_gl ("glBindVertexArray");

    glDisable (GL_DEPTH_TEST);
    glDepthMask (false);

    glDrawElements (GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);
    check_gl ("glDrawElements");

    glDepthMask (true);

    glBindVertexArray (0);
    check_gl ("glBindVertexArray");

    program.done ();
  }

}
