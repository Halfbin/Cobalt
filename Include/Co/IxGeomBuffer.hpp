//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXGEOMBUFFER
#define CO_H_IXGEOMBUFFER

#include <Rk/IxUnique.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class IxGeomBuffer :
    public Rk::IxUnique
  {
  public:
    typedef Rk::IxUniquePtr <IxGeomBuffer> Ptr;

    virtual void load_data (const void* data, uptr size) = 0;

  };

}

#endif
