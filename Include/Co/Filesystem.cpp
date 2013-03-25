//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Filesystem.hpp>

// Uses
#include <Co/Log.hpp>

#include <Rk/Modular.hpp>
#include <Rk/File.hpp>

#include <stdexcept>

namespace Co
{
  class FileImpl :
    public File
  {
    Rk::File file;

    virtual u64 seek (u64 dist, SeekMode mode)
    {
      if (mode >= seek_max_)
        throw std::invalid_argument ("Invalid seek mode");

      return file.seek (dist, (Rk::File::SeekMode) mode);
    }

    virtual u64 tell () const
    {
      return file.tell ();
    }

    virtual uptr read (void* data, uptr length)
    {
      return file.read (data, length);
    }

    virtual void write (const void* data, uptr length)
    {
      file.write (data, length);
    }

    virtual bool eof () const
    {
      return file.eof ();
    }

    virtual u64 size () const
    {
      return file.size ();
    }

  public:
    FileImpl (Rk::StringRef path, Rk::File::OpenMode mode) :
      file (path, mode)
    { }
    
  };

  class FSImpl :
    public Filesystem
  {
    Log& log;

    virtual void mount_native (Rk::StringRef mount_point, Rk::StringRef path)
    {

    }

    virtual FileIn::Ptr open_read (Rk::StringRef path)
    {
      return std::make_shared <FileImpl> (path.string (), Rk::File::open_read_existing);
    }

    virtual FileOut::Ptr open_write (Rk::StringRef path)
    {
      throw std::runtime_error ("Method not implemented");
      //return std::make_shared <FileImpl> (game_path + path.string (), Rk::File::open_replace_or_create);
    }

    virtual File::Ptr open (Rk::StringRef path)
    {
      throw std::runtime_error ("Method not implemented");
      //return std::make_shared <FileImpl> (game_path + path.string (), Rk::File::open_replace_or_create);
    }

  public:
    FSImpl (Log& log) :
      log (log)
    { }
    
  };

  class Root :
    public FilesystemRoot
  {
    virtual Filesystem::Ptr create_fs (Log& log)
    {
      return std::make_shared <FSImpl> (log);
    }

  };

  RK_MODULE (Root);

}
