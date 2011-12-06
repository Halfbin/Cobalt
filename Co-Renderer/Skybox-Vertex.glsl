//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform mat4 world_to_clip;

in vec3 attrib_pos;
//in float attrib_alpha;

out vec3 cube_dir;
//out float original_alpha;

void main ()
{
  // switch to OpenGL left-handed nonsense
  cube_dir = attrib_pos.yzx * vec3 (-1, 1, 1);

  //original_alpha = attrib_alpha;

  gl_Position = world_to_clip * vec4 (attrib_pos, 1);
}
