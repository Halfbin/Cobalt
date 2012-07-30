//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_FILESYSTEM
#define CO_H_FILESYSTEM

// Uses
#include <Rk/StringRef.hpp>
#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  enum SeekMode :
    u32
  {
    seek_absolute = 0,
    seek_set      = 0,
    seek_start    = 0,
    seek_relative = 1,
    seek_current  = 1,
    seek_end      = 2,
    
    seek_max_
  };
  
  class FileBase
  {
  public:
    typedef std::shared_ptr <FileBase> Ptr;

    virtual u64  seek  (u64 dist, SeekMode mode = seek_relative) = 0;
    virtual u64  tell  () const = 0;
    virtual bool eof   () const = 0;
    virtual u64  size  () const = 0;

  };

  class FileIn :
    virtual public FileBase
  {
  public:
    typedef std::shared_ptr <FileIn> Ptr;

    virtual uptr read (void* data, uptr length) = 0;

  };

  class FileOut :
    virtual public FileBase
  {
  public:
    typedef std::shared_ptr <FileOut> Ptr;

    virtual void write (const void* data, uptr length) = 0;

  };

  class File :
    public FileIn,
    public FileOut
  {
  public:
    typedef std::shared_ptr <File> Ptr;

  };
  
  class Filesystem
  {
  public:
    static Rk::StringRef ix_name () { return "Co::Filesystem"; }
    typedef std::shared_ptr <Filesystem> Ptr;

    virtual void init (Rk::StringRef game_path) = 0;

    virtual FileIn::Ptr  open_read  (Rk::StringRef path) = 0;
    virtual FileOut::Ptr open_write (Rk::StringRef path) = 0;
    virtual File::Ptr    open       (Rk::StringRef path) = 0;

  };

}

#endif
