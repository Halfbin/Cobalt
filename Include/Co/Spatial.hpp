//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_SPATIAL
#define CO_H_SPATIAL

#include <Rk/Quaternion.hpp>
#include <Rk/Vector3.hpp>

namespace Co
{
  typedef Rk::Vector3f    Vector3;
  typedef Rk::Quaternionf Quaternion;

  struct Spatial
  {
    Vector3    position;
    Quaternion orientation;

    Spatial operator * (float k) const
    {
      Spatial result = { position * k, orientation * k };
      return result;
    }
    
    Spatial operator + (const Spatial& rhs) const
    {
      Spatial result = { position + rhs.position, orientation + rhs.orientation };
      return result;
    }

  };

}

#endif
