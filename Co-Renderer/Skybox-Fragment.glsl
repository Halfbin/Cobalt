//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform samplerCube tex_cube;

in vec3 cube_dir;

void main ()
{
  gl_FragColor = texture (tex_cube, cube_dir);
}
