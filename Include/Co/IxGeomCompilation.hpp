//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXGEOMCOMPILATION
#define CO_H_IXGEOMCOMPILATION

#include <Rk/Types.hpp>
#include <Rk/IxUnique.hpp>

namespace Co
{
  enum AttribIndex : u8
  {
    attrib_position = 0,
    attrib_tcoords,
    attrib_normal
  };
  
  enum AttribType : u8
  {
    attrib_i8 = 0,
    attrib_i16,
    attrib_i32,
    attrib_f32
  };
  
  struct GeomAttrib
  {
    u8 index,
       type,
       stride,
       offset;
  };

  class IxGeomCompilation :
    public Rk::IxUnique
  {
  public:
    typedef Rk::IxUniquePtr <IxGeomCompilation> Ptr;

  };

} // namespace Co

#endif
