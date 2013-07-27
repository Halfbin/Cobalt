//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENTITYMANAGER
#define CO_H_ENTITYMANAGER

#include <Co/Entity.hpp>

#include <vector>

namespace Co
{
  class EntityManager
  {
    std::vector <Entity::Ptr> ents;

  public:
    void notify_all ()
    {
      for (auto iter = ents.begin (); iter != ents.end (); iter++)
        (*iter) -> notify ();
    }

    void simulate_all (float t, float dt)
    {
      for (auto iter = ents.begin (); iter != ents.end (); iter++)
        (*iter) -> simulate (t, dt);
    }

    void dispatch_all ()
    {
      for (auto iter = ents.begin (); iter != ents.end (); iter++)
        (*iter) -> dispatch ();
    }

  };

}

#endif
