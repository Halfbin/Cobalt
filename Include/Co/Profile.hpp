//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_PROFILE
#define CO_H_PROFILE

#include <Rk/VirtualOutStream.hpp>
#include <Rk/StringRef.hpp>

#include <Co/Clock.hpp>

namespace Co
{
  class Profiler :
    public Clock
  {
    Rk::StringRef               name;
    Rk::VirtualLockedOutStream& log;

  public:
    Profiler (Rk::StringRef new_name, Rk::VirtualLockedOutStream& new_log) :
      name (new_name),
      log  (new_log)
    { }
    
    ~Profiler ()
    {
      log << name << " - " << time () << "s\n";
    }

  };

}

#endif
