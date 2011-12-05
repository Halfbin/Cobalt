//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "State.hpp"

// Uses
#include <Co/IxFrame.hpp>

namespace Ir
{
  class TitleState :
    public State
  {
    Co::Spatial cam;

    virtual void enter ();
    virtual void leave ();
    virtual void tick  (float time, float prev_time);

  } title;

  State* title_state = &title;

  void TitleState::enter ()
  {
    cam.position    = nil;
    cam.orientation = nil;
  }

  void TitleState::leave ()
  {

  }

  void TitleState::tick (float time, float prev_time)
  {

  }

}
