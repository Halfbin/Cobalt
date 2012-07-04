//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENTITY
#define CO_H_ENTITY

#include <Co/UIInput.hpp>
#include <Co/Frame.hpp>

#include <memory>

namespace Co
{
  class Entity
  {
  public:
    typedef std::shared_ptr <Entity> Ptr;

    virtual void tick (Frame& frame, float time, float prev_time, const KeyState* keyboard, v2f mouse_delta) = 0;

  };

}

#endif
