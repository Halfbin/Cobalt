//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxLog.hpp>

// Uses
#include <Rk/Format.hpp>
#include <Rk/Mutex.hpp>
#include <Rk/File.hpp>

namespace Co
{
  namespace
  {
    extern "C" __declspec(dllimport) i32 __cdecl _fcvt_s (char*, uptr, double, i32, i32*, i32*);

    class Log :
      public IxLog
    {
      Rk::Mutex mutex;
      Rk::File  file;
      
      virtual void destroy ()
      {
        delete this;
      }

      virtual void write_chars (Rk::StringRef chars)
      {
        file.write (chars.data (), chars.length ());
      }
      
      virtual void write_int (u64 magnitude, bool negative, uint base, uint width)
      {
        if (base < 2 || base > 36) return;
        char buffer [66];
        uint written = Rk::format_int (magnitude, negative, base, buffer, 66, width);
        write_chars (Rk::StringRef (buffer, written));
      }
      
      virtual void write_float (long double value, uint places)
      {
        i32 point, negative;
        char buffer [66];
        i32 fail = _fcvt_s (buffer, 66, value, places, &point, &negative);
        if (fail) return;
        if (negative) write_chars ("-");
        if (point <= 0)
        {
          write_chars ("0.");
          write_chars (buffer);
        }
        else
        {
          write_chars (Rk::StringRef (buffer, point));
          write_chars (".");
          write_chars (Rk::StringRef (buffer + point));
        }
      }
      
      virtual void do_flush ()
      {
        file.flush ();
      }
      
      // Sync
      virtual void lock ()
      {
        mutex.lock ();
      }
      
      virtual void unlock ()
      {
        mutex.unlock ();
      }
      
      virtual bool try_lock ()
      {
        return mutex.try_lock ();
      }

    public:
      Log (Rk::StringRef path) :
        file (path, file.open_replace_or_create)
      { }
      
    }; // class Log

  } // namespace

  IxLog* create_log (Rk::StringRef path)
  {
    return new Log (path);
  }

}// namespace Co