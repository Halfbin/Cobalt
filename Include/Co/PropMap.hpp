//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_PROPMAP
#define CO_H_PROPMAP

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
    virtual v2f           get_v2f        (Rk::StringRef key) = 0;
    virtual v3f           get_v3f        (Rk::StringRef key) = 0;
    virtual v4f           get_v4f        (Rk::StringRef key) = 0;
    virtual v2i           get_v2i        (Rk::StringRef key) = 0;
    virtual v3i           get_v3i        (Rk::StringRef key) = 0;
    virtual v4i           get_v4i        (Rk::StringRef key) = 0;
    virtual Quat         get_quaternion (Rk::StringRef key) = 0;

  public:

  };

}

#endif
