//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_SPATIAL
#define CO_H_SPATIAL

#include <Rk/Quaternion.hpp>
#include <Rk/Vector4.hpp>
#include <Rk/Matrix.hpp>
#include <Rk/Lerp.hpp>

#include <complex>

namespace Co
{
  typedef Rk::Quaternionf      Quat;
  typedef std::complex <float> Complex;

  static inline v2f right (const Complex& ori)
  {
    return v2f (ori.real (), -ori.imag ());
  }

  static inline v2f down (const Complex& ori)
  {
    return v2f (ori.imag (), ori.real ());
  }

  static inline Complex rotate (float angle)
  {
    return Complex (std::cos (angle), std::sin (angle));
  }

  struct Spatial
  {
    v3f  position;
    Quat orientation;

    Spatial ()
    { }

    Spatial (const Nil&) :
      position    (nil),
      orientation (nil)
    { } 
    
    Spatial (v3f pos, Quat ori) :
      position    (pos),
      orientation (ori)
    { }
    
    mat4f to_matrix () const
    {
      mat4f mat;
      mat.column (0) = v4f (orientation.forward (), 0.0f);
      mat.column (1) = v4f (orientation.left (),    0.0f);
      mat.column (2) = v4f (orientation.up (),      0.0f);
      mat.column (3) = v4f (position,               1.0f);
      return mat;
    }

  }; // struct Spatial

  static inline Spatial lerp (const Spatial& a, const Spatial& b, float alpha)
  {
    return Spatial (
      Rk::lerp  (a.position,    b.position,    alpha),
      Rk::nlerp (a.orientation, b.orientation, alpha)
    );
  }

  struct Spatial2D
  {
    v2f     position;
    Complex orientation;

    Spatial2D (const Nil&) :
      position    (nil),
      orientation (1.0f, 0.0f)
    { } 
    
    Spatial2D (v2f pos = nil, Complex ori = Complex (1.0f, 0.0f)) :
      position    (pos),
      orientation (ori)
    { }
    
    mat3f to_matrix () const
    {
      mat3f mat;
      mat.column (0) = v3f (right (orientation), 0.0f);
      mat.column (1) = v3f (down  (orientation), 0.0f);
      mat.column (2) = v3f (position,            1.0f);
      return mat;
    }

  };

  static inline Spatial2D lerp (const Spatial2D& a, const Spatial2D& b, float alpha)
  {
    return Spatial2D (
      Rk::lerp (a.position,    b.position,    alpha),
      Rk::lerp (a.orientation, b.orientation, alpha)
    );
  }

}

#endif
