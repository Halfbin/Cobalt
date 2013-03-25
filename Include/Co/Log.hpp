//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LOG
#define CO_H_LOG

#include <Rk/FileOutStream.hpp>

#include <exception>

namespace Co
{
  typedef Rk::SyncOutStream <Rk::OutStream> Log;

  class LogFile
  {
    Rk::FileOutStream file;

  public:
    LogFile (Rk::StringRef img_name, bool append) :
      file (img_name.string () + ".log", append ? Rk::File::open_append_or_create : Rk::File::open_replace_or_create)
    { }
    
    Rk::OutStream stream ()
    {
      return file.virtualize ();
    }

  };

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
