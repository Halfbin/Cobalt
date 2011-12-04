//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXENGINE
#define CO_H_IXENGINE

#include <Co/Library.hpp>

#include <Rk/StringRef.hpp>

namespace Co
{
  class IxRenderer;
  class IxLoader;
  class IxEntityClass;
  class IxPropMap;
  class IxModule;
  class IxEntity;
  class Clock;

  class IxEngine
  {
  public:
    static const u64 id = 0xbafa9de9d2a72156ull;

    virtual bool init (IxRenderer* renderer, IxLoader* loader, Clock* clock) = 0;

    virtual IxModule* load_module (Rk::StringRef name) = 0;

    virtual void register_classes   (IxEntityClass** classes, IxEntityClass** end) = 0;
    virtual void unregister_classes (IxEntityClass** classes, IxEntityClass** end) = 0;

    void register_classes (const Library* lib)
    {
      register_classes (lib -> begin (), lib -> end ());
    }

    void unregister_classes (const Library* lib)
    {
      unregister_classes (lib -> begin (), lib -> end ());
    }

    virtual IxEntity* create_entity (Rk::StringRef type, IxPropMap* props) = 0;
    virtual void      open          (Rk::StringRef scene) = 0;

    virtual float start     () = 0;
    virtual void  wait      () = 0;
    virtual bool  update    (float& next_update) = 0;
    virtual void  terminate () = 0;

  }; // class IxEngine

} // namespace Co

#endif
