//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENTITY
#define CO_H_ENTITY

#include <memory>

namespace Co
{
  class Entity
  {
  public:
    typedef std::shared_ptr <Entity> Ptr;

    virtual void notify   () = 0;
    virtual void simulate (float t, float dt) = 0;
    virtual void dispatch () = 0;

  };

  /*
  TICK
    for each ent
      notify

    add time - t to accumulator

    while accumuator >= dt
      t += dt
      for each ent
        simulate to t by dt
      accumulator -= dt

    for each ent
      dispatch
  */

}

#endif
