//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXMODULE
#define CO_H_IXMODULE

namespace Co
{
  class IxModule
  {
  public:
    virtual void* expose (u64 ixid) = 0;

    template <typename Ix>
    Ix* expose ()
    {
      return (Ix*) expose (Ix::id);
    }
    
    template <typename Ix>
    void expose (Ix*& ix)
    {
      ix = expose <Ix> ();
    }

  }; // class IxModule

} // namespace Co

#endif
