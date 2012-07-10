//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_ENTITY
#define CO_H_ENTITY

#include <Co/WorkQueue.hpp>
#include <Co/UIInput.hpp>
#include <Co/Frame.hpp>

#include <memory>

namespace Co
{
  class Entity
  {
  public:
    typedef std::shared_ptr <Entity> Ptr;

    virtual void tick   (float time, float step, WorkQueue& queue, const KeyState* keyboard, v2f mouse_delta) = 0;
    virtual void render (Frame& frame, float alpha) = 0;

  };

}

#endif
