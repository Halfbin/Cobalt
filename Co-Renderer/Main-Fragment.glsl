//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#version 330 core

uniform sampler2D tex_diffuse,
                  tex_specular,
                  tex_emission,
                  tex_exponent,
                  tex_normal;

uniform vec4  mat_ambient,
              mat_diffuse,
              mat_specular,
              mat_emissive;
uniform float mat_exponent;

in vec4 xformed_position;
in vec4 xformed_normal;
in vec2 xformed_tcoords;

void main ()
{
  vec4 n = xformed_normal;
  
  vec4 texel_diffuse  = texture (tex_diffuse,  xformed_tcoords),
       texel_specular = texture (tex_specular, xformed_tcoords),
       texel_emission = texture (tex_emission, xformed_tcoords),
       texel_exponent = texture (tex_exponent, xformed_tcoords),
       texel_normal   = texture (tex_normal,   xformed_tcoords);

  vec4  ref_ambient  = mat_ambient,
        ref_diffuse  = mat_diffuse,
        ref_specular = mat_specular,
        ref_emissive = mat_emissive;
  float ref_exponent = mat_exponent;

  gl_FragColor = texel_diffuse;
}
