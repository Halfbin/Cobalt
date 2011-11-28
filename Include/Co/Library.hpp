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
    IxEntityClass** classes;
    IxEntityClass** classes_end;

  public:
    static const u64 id = 0xe08de5f30718514cull;

    template <uint count>
    Library (IxEntityClass* (&new_classes) [count]) :
      classes     (new_classes),
      classes_end (new_classes + count)
    { }
    
    IxEntityClass** begin () const
    {
      return classes;
    }

    IxEntityClass** end () const
    {
      return classes_end;
    }

  };

}

#endif
