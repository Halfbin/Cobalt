//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXENGINE
#define CO_H_IXENGINE

#include <Rk/StringRef.hpp>
#include <Co/Clock.hpp>

namespace Co
{
  class IxRenderer;
  class IxLoader;
  class IxEntityClass;
  class IxEntity;
  class IxPropMap;
  class IxGame;
  
  class IxEngine
  {
  public:
    enum : u64 { id = 0xbafa9de9d2a72156ull };

    virtual void init (IxRenderer* renderer, IxLoader* loader, Clock* clock) = 0;

    virtual void register_classes   (IxEntityClass* classes, uint count) = 0;
    virtual void unregister_classes (IxEntityClass* classes, uint count) = 0;

    virtual IxEntity* create_entity (u64 type, IxPropMap* props, IxEntity* owner) = 0;
    virtual void      open          (Rk::StringRef scene) = 0;

    virtual void run (IxGame* game) = 0;
    virtual void terminate () = 0;

  };

}

#endif
