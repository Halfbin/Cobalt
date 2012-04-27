//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LOG
#define CO_H_LOG

#include <Rk/OutStream.hpp>

#include <exception>

namespace Co
{
  typedef Rk::SyncOutStream <Rk::OutStream> Log;

  static inline void log_exception (Log& log, Rk::StringRef source)
  {
    try { throw; }
    catch (...)
    {
      auto lock = log.lock ();
      lock << "X Exception caught";
      if (source)
        lock << ' ' << source;
      lock << '\n';

      try { throw; }
      catch (const std::exception& e)
      {
        Rk::StringRef message = e.what ();
        if (message)
          lock << "  " << message << '\n';
      }
    }
  }

}

#endif
