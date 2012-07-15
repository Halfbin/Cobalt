//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform mat4 model_to_clip;

in vec3 attrib_position;
in vec3 attrib_tcoords;
in vec3 attrib_colour;

out vec4 xformed_position;
out vec3 xformed_tcoords;
out vec4 xformed_colour;

void main ()
{
  xformed_position = model_to_clip * vec4 (attrib_position, 1);
  
  xformed_tcoords = attrib_tcoords;
  xformed_colour = vec4 (attrib_colour, 1);

  gl_Position = xformed_position;
}
