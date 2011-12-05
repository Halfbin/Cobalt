//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXENGINE
#define CO_H_IXENGINE

#include <Rk/StreamForward.hpp>
#include <Rk/StringRef.hpp>
#include <Rk/IxUnique.hpp>

namespace Co
{
  class IxRenderer;
  class IxLoader;
  class IxEntityClass;
  class IxPropMap;
  class IxModule;
  class IxEntity;
  class Clock;

  class IxEngine :
    public Rk::IxUnique
  {
  public:
    static const u64 id = 0xbafa9de9d2a72156ull;

    typedef Rk::IxUniquePtr <IxEngine> Ptr;

    virtual bool init (IxRenderer* renderer, IxLoader* loader, Clock* clock, Rk::IxLockedOutStreamImpl* log) = 0;
    
    virtual IxModule* load_module (Rk::StringRef name) = 0;
    virtual u32       clear_modules () = 0;

    virtual bool register_classes   (const IxEntityClass* const* classes, const IxEntityClass* const* end) = 0;
    virtual bool unregister_classes (const IxEntityClass* const* classes, const IxEntityClass* const* end) = 0;

    virtual IxEntity* create_entity (Rk::StringRef type, IxPropMap* props) = 0;
    virtual bool      open          (Rk::StringRef scene) = 0;

    virtual bool start     (float& new_time) = 0;
    virtual void wait      () = 0;
    virtual bool update    (float& next_update) = 0;
    virtual void terminate () = 0;

    template <typename Container>
    bool register_classes (const Container& cont)
    {
      return register_classes (std::begin (cont), std::end (cont));
    }

    template <typename Container>
    bool unregister_classes (const Container& cont)
    {
      return unregister_classes (std::begin (cont), std::end (cont));
    }

  }; // class IxEngine

} // namespace Co

#endif
