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
      blend_colour.rgb,
      pow (texture (tex_cube, cube_dir).rgb, gamma),
      pow (blend_colour.a, gamma)
    ),
    1
  );
}
