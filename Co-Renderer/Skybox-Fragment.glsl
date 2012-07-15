//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform samplerCube tex_cube;
uniform vec4        blend_colour;

in vec3 cube_dir;

vec3 pow (in vec3 a, in float ex)
{
  return vec3 (
    pow (a.x, ex),
    pow (a.y, ex),
    pow (a.z, ex)
  );
}

const float gamma = 2.2f;

void main ()
{
  gl_FragColor = vec4 (
    mix (
      pow (blend_colour.rgb, gamma),
      texture (tex_cube, cube_dir).rgb,
      blend_colour.a
    ),
    1
  );
}
