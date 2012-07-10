//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_MODULE
#define CO_H_MODULE

#include <memory>

namespace Co
{
  class Module
  {
  public:
    typedef std::shared_ptr <Module> Ptr;

    //virtual void* expose (u64 ixid) = 0;

  };

}

#endif
