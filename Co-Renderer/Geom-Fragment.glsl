//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform sampler2D tex_diffuse;/*,
                  tex_specular,
                  tex_emission,
                  tex_exponent,
                  tex_normal;*/

uniform vec4  //mat_ambient,
              mat_diffuse/*,
              mat_specular,
              mat_emissive;
uniform float mat_exponent*/;

in vec4 xformed_position;
//in vec4 xformed_normal;
in vec2 xformed_tcoords;
in vec4 xformed_colour;

vec4 pow (in vec4 val, in float ex)
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
  //vec4 n = xformed_normal;
  
  vec4 texel_diffuse  = /*pow (*/texture (tex_diffuse,  xformed_tcoords)/*, gamma)*/;/*,
       texel_specular = texture (tex_specular, xformed_tcoords),
       texel_emission = texture (tex_emission, xformed_tcoords),
       texel_exponent = texture (tex_exponent, xformed_tcoords),
       texel_normal   = texture (tex_normal,   xformed_tcoords);*/

  /*vec4  ref_ambient  = mat_ambient,
        ref_diffuse  = mat_diffuse,
        ref_specular = mat_specular,
        ref_emissive = mat_emissive;
  float ref_exponent = mat_exponent;*/

  gl_FragColor = pow (xformed_colour, gamma) * texel_diffuse;
}
