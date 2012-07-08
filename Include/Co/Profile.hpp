//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_PROFILE
#define CO_H_PROFILE

#include <Co/Clock.hpp>
#include <Co/Log.hpp>

namespace Co
{
  class Profiler :
    public Clock
  {
    Rk::StringRef name;
    Log&          log;

  public:
    Profiler (Rk::StringRef new_name, Log& new_log) :
      name (new_name),
      log  (new_log)
    { }
    
    void done ()
    {
      log () << "- " << name << " - " << time () << "s\n";
    }

  };

}

#endif
