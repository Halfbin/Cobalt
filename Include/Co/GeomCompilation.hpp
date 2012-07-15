//
// Copyright (C) 2012 Roadkill Software
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

  enum IndexType : u32
  {
    index_none = 0,
    index_u8,
    index_u16,
    index_u32
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
