//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LIBRARY
#define CO_H_LIBRARY

#include <Rk/Expose.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class IxEntityClass;

  class Library
  {
    const IxEntityClass** classes;
    const IxEntityClass** classes_end;

  public:
    static const u64 id = 0xe08de5f30718514cull;

    template <uint count>
    Library (const IxEntityClass* (&new_classes) [count]) :
      classes     (new_classes),
      classes_end (new_classes + count)
    { }
    
    const IxEntityClass** begin () const
    {
      return classes;
    }

    const IxEntityClass** end () const
    {
      return classes_end;
    }

    void expose (void** out, u64 ixid)
    {
      Rk::expose <Library> (this, ixid, out);
    }

  };

}

#endif
