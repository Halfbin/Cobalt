//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

/*uniform mat4 model_to_world;
uniform mat4 world_to_clip;
uniform mat4 world_to_eye;*/

uniform mat4 model_to_clip;
//uniform mat4 model_to_eye;

in vec3 attrib_position;
//in vec3 attrib_normal;
in vec2 attrib_tcoords;
in vec3 attrib_colour;

out vec4 xformed_position;
//out vec4 xformed_normal;
out vec2 xformed_tcoords;
out vec4 xformed_colour;

void main ()
{
  xformed_position = model_to_clip * vec4 (attrib_position, 1);
  //xformed_normal   = world_to_eye  * model_to_world * vec4 (attrib_normal,   0);

  /*const float eps = 1.0f / 1024.0f;
  const vec2 tex_adj [4] = vec2 [4] (
    vec2 ( eps,  eps),
    vec2 ( eps, -eps),
    vec2 (-eps,  eps),
    vec2 (-eps, -eps)
  );*/

  xformed_tcoords = attrib_tcoords/* + tex_adj [gl_VertexID % 4]*/;
  xformed_colour = vec4 (attrib_colour, 1);

  gl_Position = xformed_position;
}
