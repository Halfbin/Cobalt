//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENTITYCLASS
#define CO_H_ENTITYCLASS

#include <Co/IxEntityClass.hpp>
//#include <Rk/Meta.hpp>

namespace Co
{
  /*namespace EntityClassPrivate
  {
    template <typename M, M m>
    struct Test;

    template <typename E>
    struct HasPreloadMethod
    {
      template <typename T>
      static Rk::YesType check (T*, Test <void (*) (IxLoadContext&), &T::preload>* = 0);

      template <typename T>
      static Rk::NoType check (...);

      enum { value = check ((E*) 0)::value };

    };

  };*/

  template <typename E>
  class EntityClass :
    public IxEntityClass
  {
    virtual IxEntity* create (IxLoadContext& context, IxPropMap* props) const
    {
      return new E (context, props);
    }

  public:
    EntityClass (Rk::StringRef name) :
      IxEntityClass (name)
    { }
    
  };

}

#endif
