//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENGINE
#define CO_H_ENGINE

#include <Co/WorkQueue.hpp>
#include <Co/Renderer.hpp>
#include <Co/PropMap.hpp>
#include <Co/UIInput.hpp>
#include <Co/Entity.hpp>
#include <Co/Game.hpp>
#include <Co/Host.hpp>
#include <Co/Log.hpp>

#include <Rk/StringRef.hpp>

#include <typeinfo>

namespace Co
{
  class Engine
  {
  public:
    static Rk::StringRef ix_name () { return "Co::Engine"; }
    typedef std::shared_ptr <Engine> Ptr;

    virtual void  init        (Host&, Renderer::Ptr, WorkQueue&, Game::Ptr) = 0;
    virtual float start       () = 0;
    virtual void  post_events (const UIEvent* events, const UIEvent* end) = 0;
    virtual void  wait        () = 0;
    virtual bool  update      (float& time, uint width, uint height, const UIEvent* ui_events, uint ui_event_count, const KeyState* keyboard, v2f mouse_delta) = 0;
    virtual void  stop        () = 0;
    virtual void  enable_ui   (bool enable) = 0;

    virtual Entity::Ptr create_entity (Rk::StringRef type, const PropMap* props = 0) = 0;
    
    virtual std::shared_ptr <void> get_object (Rk::StringRef type) = 0;

    template <typename T>
    std::shared_ptr <T> get_object ()
    {
      return std::static_pointer_cast <T> (
        get_object (T::ix_name ())
      );
    }

    template <typename T>
    void get_object (std::shared_ptr <T>& ptr)
    {
      ptr = get_object <T> ();
    }

  };

} // namespace Co

#endif
