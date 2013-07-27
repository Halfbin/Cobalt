//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef PYR_SERVER_H_TESTENTITY
#define PYR_SERVER_H_TESTENTITY

#include <Co/NetEntity.hpp>

namespace Pyr
{
  class TestEntity :
    public Co::NetEntity
  {
  public:
    TestEntity ();
    ~TestEntity ();

    virtual void replicate (Co::NetOutChannel&);

  };

}

#endif
