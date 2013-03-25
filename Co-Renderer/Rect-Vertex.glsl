//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform mat3 ui_to_clip;
uniform mat3 ui_to_ui;

in ivec4 attrib_rect;
in ivec4 attrib_tcoords;

smooth out vec4 xformed_position;
smooth out vec2 xformed_tcoords;

//  |  /|
//  | / |
//  |/  |

void main ()
{
  ivec4 permute [4] = ivec4 [4] (
    ivec4 (attrib_rect.x, attrib_rect.y, attrib_tcoords.s, attrib_tcoords.t),
    ivec4 (attrib_rect.x, attrib_rect.w, attrib_tcoords.s, attrib_tcoords.q),
    ivec4 (attrib_rect.z, attrib_rect.y, attrib_tcoords.p, attrib_tcoords.t),
    ivec4 (attrib_rect.z, attrib_rect.w, attrib_tcoords.p, attrib_tcoords.q)
  );

  ivec4 rect = permute [gl_VertexID];

  ivec3 ui_xy = ivec3 (ui_to_ui * vec3 (rect.xy, 1));
  xformed_position = vec4 ((ui_to_clip * vec3 (ui_xy)).xy, 0, 1);
  xformed_tcoords  = rect.zw;

  gl_Position = xformed_position;
}
