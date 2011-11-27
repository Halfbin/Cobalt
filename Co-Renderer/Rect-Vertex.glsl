//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform mat3x2 ui_to_clip;

in ivec4 attrib_rect;
in ivec4 attrib_tcoords;

out vec4  xformed_position;
out ivec2 xformed_tcoords;

//  |  /|
//  | / |
//  |/  |

void main ()
{
  int x, y, s, t;

  switch (gl_VertexID)
  {
    case 0: x = attrib_rect.x; y = attrib_rect.y; s = attrib_tcoords.s; t = attrib_tcoords.t; break;
    case 1: x = attrib_rect.x; y = attrib_rect.w; s = attrib_tcoords.s; t = attrib_tcoords.q; break;
    case 2: x = attrib_rect.z; y = attrib_rect.y; s = attrib_tcoords.p; t = attrib_tcoords.t; break;
    case 3: x = attrib_rect.z; y = attrib_rect.w; s = attrib_tcoords.p; t = attrib_tcoords.q; break;
  }

  xformed_position = vec4 (ui_to_clip * vec3 (x, y, 1), 0, 1);
  xformed_tcoords  = ivec2 (s, t);

  gl_Position = xformed_position;
}
