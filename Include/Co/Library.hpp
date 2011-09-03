//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LIBRARY
#define CO_H_LIBRARY

#include <Rk/Types.hpp>

namespace Co
{
  class IxEntityClass;

  class Library
  {
  public:
    static const u64 id = 0xe08de5f30718514cull;

    IxEntityClass** classes;
    uint            class_count;

    template <uint count>
    Library (IxEntityClass* (&array) [count]) :
      classes     (array),
      class_count (count)
    { }
    
  };

}

#endif
