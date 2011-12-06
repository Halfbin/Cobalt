//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXENTITYCLASS
#define CO_H_IXENTITYCLASS

#include <Rk/StringRef.hpp>
#include <Rk/IxUnique.hpp>

namespace Co
{
  class IxPropMap;
  class IxLoadContext;
  class IxEntity;

  class IxEntityClass
  {
  public:
    Rk::StringRef name;

    virtual IxEntity* create (IxLoadContext* context, IxPropMap* props) const = 0;

  protected:
    IxEntityClass (Rk::StringRef new_name) :
      name (new_name)
    { }
    
  };

}

#endif
