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
  typedef Rk::Vector2f         Vector2;
  typedef Rk::Vector3f         Vector3;
  typedef Rk::Vector4f         Vector4;
  typedef Rk::Quaternionf      Quaternion;
  typedef Rk::Matrix2f         Matrix2;
  typedef Rk::Matrix3f         Matrix3;
  typedef Rk::Matrix4f         Matrix4;
  typedef std::complex <float> Complex;

  static inline Vector2 right (const Complex& ori)
  {
    return Vector2 (ori.real (), -ori.imag ());
  }

  static inline Vector2 down (const Complex& ori)
  {
    return Vector2 (ori.imag (), ori.real ());
  }

  static inline Complex rotate (float angle)
  {
    return Complex (std::cos (angle), std::sin (angle));
  }

  struct Spatial
  {
    Vector3    position;
    Quaternion orientation;

    Spatial ()
    { }

    Spatial (const Nil&) :
      position    (nil),
      orientation (nil)
    { } 
    
    Spatial (Vector3 pos, Quaternion ori) :
      position    (pos),
      orientation (ori)
    { }
    
    Matrix4 to_matrix () const
    {
      Matrix4 mat;
      mat.column (0) = Vector4 (orientation.forward (), 0.0f);
      mat.column (1) = Vector4 (orientation.left (),    0.0f);
      mat.column (2) = Vector4 (orientation.up (),      0.0f);
      mat.column (3) = Vector4 (position,               1.0f);
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
    Vector2 position;
    Complex orientation;

    Spatial2D (const Nil&) :
      position    (nil),
      orientation (1.0f, 0.0f)
    { } 
    
    Spatial2D (Vector2 pos = nil, Complex ori = Complex (1.0f, 0.0f)) :
      position    (pos),
      orientation (ori)
    { }
    
    Matrix3 to_matrix () const
    {
      Matrix3 mat;
      mat.column (0) = Vector3 (right (orientation), 0.0f);
      mat.column (1) = Vector3 (down  (orientation), 0.0f);
      mat.column (2) = Vector3 (position,            1.0f);
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
