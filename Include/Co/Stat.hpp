//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_STAT
#define CO_H_STAT

#include <stdexcept>

namespace Co
{
  class Stat
  {
    const char* message;
    bool        logic;

  public:
    Stat (const char* message, bool logic) :
      message (message),
      logic   (logic)
    { }
    
    void check ()
    {
      if (message)
      {
        if (logic)
          throw std::logic_error (message);
        else
          throw std::runtime_error (message);
      }
    }

  };

}

#endif
