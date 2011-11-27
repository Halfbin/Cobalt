//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

in vec4  xformed_position;
in ivec2 xformed_tcoords;

uniform sampler2DRect tex;

void main ()
{
  vec4 colour = texelFetch (tex, xformed_tcoords);
  gl_FragColor = vec4 (colour.rrr, 1);
}
