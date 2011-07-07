//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXTHINKER
#define CO_H_IXTHINKER

namespace Co
{
  class IxThinker
  {
  public:
    virtual void think (float time, float prev_time) = 0;

  };

}

#endif
