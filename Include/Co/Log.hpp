//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LOG
#define CO_H_LOG

#include <Rk/Stream.hpp>
#include <Rk/Mutex.hpp>
#include <Rk/File.hpp>
#include <Rk/StringRef.hpp>

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
    public Rk::IxLockedOutStream
  {
    Rk::Mutex mutex;
    Rk::File  file;
    char      buf [64];

    // No copy or move
    Log             (const Log&) { }
    Log& operator = (const Log&) { }

    virtual void write_chars (Rk::StringRef chars)
    {
      file.write (chars.data (), chars.length ());
    }
    
    virtual void write_int (u64 magnitude, bool negative, uint base)
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
    
    virtual void flush ()
    {
      file.flush ();
    }
    
  public:
    Log (const char* path) :
      file (path, file.open_replace_or_create)
    { }
    
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
    
    // No copy or move
    //Log             (const Log&) = delete;
    //Log& operator = (const Log&) = delete;
    
    /*template <typename ...P>
    void operator () (P... params)
    {
      auto lock = get_lock ();
      IxOutStream::operator () (std::forward <P> (params) ..., '\n');
      flush ();
    }*/
    
  };

}

#endif
