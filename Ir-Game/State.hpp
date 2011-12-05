//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef IR_H_STATE
#define IR_H_STATE

namespace Ir
{
  class State
  {
  public:
    virtual void enter () = 0;
    virtual void leave () = 0;
    virtual void tick  (float time, float prev_time) = 0;

  };

  extern State *title_state;

}

#endif
