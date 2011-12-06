//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXENTITY
#define CO_H_IXENTITY

#include <Rk/IxUnique.hpp>

namespace Co
{
  class IxFrame;

  class IxEntity :
    public Rk::IxUnique
  {
  public:
    typedef Rk::IxUniquePtr <IxEntity> Ptr;

    virtual void tick (IxFrame* frame, float time, float prev_time) = 0;

  };

}

#endif
