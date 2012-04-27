//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_HOST
#define CO_H_HOST

// Uses
#include <Co/Log.hpp>

#include <Rk/StringRef.hpp>

#include <memory>

namespace Co
{
  class Host
  {
  protected:
    Host (const Clock& clock, Log& log) :
      clock (clock),
      log   (log)
    { }
    
  public:
    const Clock& clock;
    Log&         log;

    virtual std::shared_ptr <void> get_object (Rk::StringRef name) = 0;

    virtual void enable_ui (bool enabled) = 0;
    
  };

}

#endif
