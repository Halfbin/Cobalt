//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

smooth in vec4 xformed_position;
smooth in vec2 xformed_tcoords;

uniform sampler2DRect tex;
uniform vec4          linear_colour;
uniform vec4          const_colour;

out vec4 frag;

vec3 vpow (in vec3 val, in float ex)
{
  return vec3 (
    pow (val.x, ex),
    pow (val.y, ex),
    pow (val.z, ex)
  );
}

const float gamma = 2.2f;

void main ()
{
  vec4 texel    = texelFetch (tex, ivec2 (xformed_tcoords));
  vec4 linear   = linear_colour * texel;//vec4 (/*pow (*/texel.rgb/*, gamma)*/, texel.a);
  vec4 constant = vec4 (vpow (const_colour.rgb, gamma), 1.0f);
  frag = mix (linear, constant, const_colour.a);
}
