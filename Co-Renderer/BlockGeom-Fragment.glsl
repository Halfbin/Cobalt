//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform sampler2DArray tex_array;

in vec4 xformed_position;
in vec3 xformed_tcoords;
in vec4 xformed_colour;

out vec4 frag;

vec4 vpow (in vec4 val, in float ex)
{
  return vec4 (
    pow (val.x, ex),
    pow (val.y, ex),
    pow (val.z, ex),
    pow (val.w, ex)
  );
}

const float gamma = 2.2f;

void main ()
{
  vec4 texel = texture (tex_array,  xformed_tcoords);
  frag = vpow (xformed_colour, gamma) * texel;
}
