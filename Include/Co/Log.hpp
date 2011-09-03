//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LOG
#define CO_H_LOG

#include <Rk/VirtualOutStream.hpp>
#include <Rk/NoCopy.hpp>
#include <Rk/Mutex.hpp>
#include <Rk/File.hpp>

namespace Co
{
  namespace LogPrivate
  {
    extern "C"
    {
      __declspec(dllimport) i32 __cdecl _ui64toa_s (u64, char*, uptr, i32);
      __declspec(dllimport) i32 __cdecl _fcvt_s    (char*, uptr, double, i32, i32*, i32*);
    }
    
  }

  class Log :
    public Rk::IxLockedOutStreamImpl,
    public Rk::LockedOutStreamInterface <char, Log>
  {
    typedef Rk::LockedOutStreamInterface <char, Log>
      Interface;
    
    Rk::Mutex mutex;
    Rk::File  file;
    char      buf [64];

    // No copy or move
    Rk::NoCopy no_copy;
    
  public:
    #pragma warning (push)
    #pragma warning (disable: 4355)
      Log (const char* path) :
        Interface (*this),
        file      (path, file.open_replace_or_create)
      { }
    #pragma warning (pop)
    
    virtual void write_chars (Rk::StringRef chars)
    {
      file.write (chars.data (), chars.length ());
    }
    
    virtual void write_int (u64 magnitude, bool negative, uint base, uint width)
    {
      if (base < 2 || base > 36) return;
      char* p = buf;
      if (negative) *p++ = '-';
      i32 fail = LogPrivate::_ui64toa_s (magnitude, p, 62, base);
      if (!fail) write_chars (buf);
    }
    
    virtual void write_float (long double value, uint places)
    {
      i32 point, negative;
      i32 fail = LogPrivate::_fcvt_s (buf, 62, value, places, &point, &negative);
      if (fail) return;
      if (negative  ) write_chars ("-");
      if (point <= 0)
      {
        write_chars ("0.");
        write_chars (buf);
      }
      else
      {
        write_chars (Rk::StringRef (buf, point));
        write_chars (".");
        write_chars (Rk::StringRef (buf + point));
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
    
  }; // class Log

} // namespace Co

#endif
