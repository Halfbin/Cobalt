//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "Noise.hpp"

namespace SH
{
  v2f noise_grads_2d [256];

  const float phi = 0.5f * (1.0f + std::sqrt (5.0f));

  v3f noise_grads_3d [32] = {
    v3f (1, phi, 0), v3f (-1, phi, 0), v3f (1, -phi, 0), v3f (-1, -phi, 0),
    v3f (0, 1, phi), v3f (0, -1, phi), v3f (0, 1, -phi), v3f (0, -1, -phi),
    v3f (phi, 0, 1), v3f (phi, 0, -1), v3f (-phi, 0, 1), v3f (-phi, 0, -1),
    v3f (phi, 1, 0), v3f (-phi, 1, 0), v3f (phi, -1, 0), v3f (-phi, -1, 0),
    v3f (0, phi, 1), v3f (0, -phi, 1), v3f (0, phi, -1), v3f (0, -phi, -1),
    v3f (1, 0, phi), v3f (1, 0, -phi), v3f (-1, 0, phi), v3f (-1, 0, -phi),
    v3f (1, 1, 1),   v3f (-1, 1, 1),   v3f (1, -1, 1),   v3f (-1, -1, 1),
    v3f (1, 1, -1),  v3f (-1, 1, -1),  v3f (1, -1, -1),  v3f (-1, -1, -1)
  };

  struct Init
  {
    Init ()
    {
      // Generate even distributions of unit vectors in n-sphere

      // In 2 dimensions this is easy
      const float theta = (2.0f * 3.14152f / 256.0f);
      for (uint i = 0; i != 256; i++)
        noise_grads_2d [i] = v2f (std::cos (i * theta), std::sin (i * theta));

      // Normalize 3d grads
      for (uint i = 0; i != 32; i++)
        normalize (noise_grads_3d [i]);
    }

  } static init;

}
