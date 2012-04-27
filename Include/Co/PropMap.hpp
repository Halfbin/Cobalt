//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_PROFILE
#define CO_H_PROFILE

#include <Co/Spatial.hpp>

#include <Rk/StringRef.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class PropMap
  {
    virtual Rk::StringRef get_string     (Rk::StringRef key) = 0;
    virtual uint          get_uint       (Rk::StringRef key) = 0;
    virtual int           get_int        (Rk::StringRef key) = 0;
    virtual float         get_float      (Rk::StringRef key) = 0;
    virtual Vector3       get_vector3    (Rk::StringRef key) = 0;
    virtual Vector4       get_vector4    (Rk::StringRef key) = 0;
    virtual Quaternion    get_quaternion (Rk::StringRef key) = 0;

  public:

  };

}

#endif
