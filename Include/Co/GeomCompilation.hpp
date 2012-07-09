//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXGEOMCOMPILATION
#define CO_H_IXGEOMCOMPILATION

#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  enum AttribIndex : u8
  {
    attrib_position = 0,
    attrib_tcoords,
    attrib_normal,
    attrib_colour
  };
  
  enum AttribType : u8
  {
    attrib_i8 = 0,
    attrib_u8,
    attrib_i16,
    attrib_i32,
    attrib_f32,

    attrib_norm_begin_ = attrib_f32,

    attrib_i8n,
    attrib_u8n
  };
  
  struct GeomAttrib
  {
    u8 index,
       type,
       stride,
       offset;
  };

  class GeomCompilation
  {
  public:
    typedef std::shared_ptr <GeomCompilation> Ptr;

  };

} // namespace Co

#endif
