//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Filesystem.hpp>

// Uses
#include <Rk/Module.hpp>
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

    virtual u32 read (void* data, u32 length)
    {
      return file.read (data, length);
    }

    virtual void write (const void* data, u32 length)
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
    std::string game_path;

    virtual void init (Rk::StringRef new_game_path)
    {
      game_path = new_game_path.string ();
    }

    virtual FileIn::Ptr open_read (Rk::StringRef path)
    {
      return std::make_shared <FileImpl> (game_path + path.string (), Rk::File::open_read_existing);
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
    FSImpl ()
    { }

    static Ptr create ()
    {
      return std::make_shared <FSImpl> ();
    }

  };

  RK_MODULE_FACTORY (FSImpl);

}
