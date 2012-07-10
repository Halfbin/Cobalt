//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENTITYCLASS
#define CO_H_ENTITYCLASS

// Uses
#include <Co/WorkQueue.hpp>
#include <Co/PropMap.hpp>
#include <Co/Entity.hpp>

#include <Rk/StringRef.hpp>

namespace Co
{
  class EntityClassBase
  {
  public:
    virtual Rk::StringRef name   () const = 0;
    virtual Entity::Ptr   create (WorkQueue& queue, const PropMap* props) const = 0;

  };

  /*template <typename EntType>
  Entity::Ptr create_entity (WorkQueue& queue, const PropMap& props)
  {
    return return Entity::Ptr (
      new EntType (queue, props),
      queue.make_deleter <EntType> ()
    );
  }*/

  template <typename EntType>
  class EntityClass :
    public EntityClassBase
  {
    const Rk::StringRef name_;

    virtual Entity::Ptr create (WorkQueue& queue, const PropMap* props) const
    {
      return EntType::create (queue, props);
    }

    virtual Rk::StringRef name () const
    {
      return name_;
    }

  public:
    EntityClass (Rk::StringRef name) :
      name_ (name)
    { }
    
  };

}

#endif
