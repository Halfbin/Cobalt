//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

smooth in vec4 xformed_position;
smooth in vec2 xformed_tcoords;

uniform sampler2DRect tex;
uniform mat4          tex_to_colour;

void main ()
{
  vec4 sample = texelFetch (tex, ivec2 (xformed_tcoords));
  gl_FragColor = tex_to_colour * sample;
}
