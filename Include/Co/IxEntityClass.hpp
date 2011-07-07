//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXENTITYCLASS
#define CO_H_IXENTITYCLASS

namespace Co
{
  class IxPropMap;
  class IxEntImpl;

  class IxEntityClass
  {
  public:
    u64 id;

    virtual IxEntImpl* create (IxPropMap* props, IxEntity* parent) = 0;

  };

}

#endif
