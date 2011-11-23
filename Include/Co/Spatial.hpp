//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_SPATIAL
#define CO_H_SPATIAL

#include <Rk/Quaternion.hpp>
#include <Rk/Vector4.hpp>

namespace Co
{
  typedef Rk::Vector3f    Vector3;
  typedef Rk::Vector4f    Vector4;
  typedef Rk::Quaternionf Quaternion;

  struct Spatial
  {
    Vector3    position;
    Quaternion orientation;

    Spatial ()
    { }

    Spatial (Vector3 pos, Quaternion ori) :
      position    (pos),
      orientation (ori)
    { }
    
    /*Spatial operator * (float k) const
    {
      Spatial result (position * k, orientation * k);
      return result;
    }
    
    Spatial operator + (const Spatial& rhs) const
    {
      Spatial result (position + rhs.position, orientation + rhs.orientation);
      return result;
    }*/
    
  }; // struct Spatial

  static inline Spatial lerp (const Spatial& a, const Spatial& b, float alpha)
  {
    return Spatial (
      Rk::lerp  (a.position,    b.position,    alpha),
      Rk::nlerp (a.orientation, b.orientation, alpha)
    );
  }

}

#endif
