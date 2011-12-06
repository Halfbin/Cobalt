//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef IR_H_STATE
#define IR_H_STATE

#include <Co/IxLoadContext.hpp>
#include <Co/IxFrame.hpp>

namespace Ir
{
  class State
  {
  public:
    virtual void enter () = 0;
    virtual void leave () = 0;
    virtual void tick  (Co::IxFrame* frame, float time, float prev_time) = 0;

  };

  extern State *title_state;

}

#endif
