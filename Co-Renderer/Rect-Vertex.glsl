//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform mat3x2 ui_to_world;

struct Rect
{
  int x, y, x2, y2,
      s, t, s2, t2;
};

in Rect attrib_position;

out vec4 xformed_position;
out vec2 xformed_tcoords;

//  |  /|
//  | / |
//  |/  |

void main ()
{
  

  gl_Position = xformed_position;
}
