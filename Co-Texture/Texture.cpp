//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Texture.hpp>

// Uses
#include <Co/ResourceFactory.hpp>
#include <Co/RenderContext.hpp>
#include <Co/TextureFile.hpp>
#include <Co/Filesystem.hpp>
#include <Co/WorkQueue.hpp>

#include <Rk/AsyncMethod.hpp>
#include <Rk/ChunkLoader.hpp>
#include <Rk/ByteStream.hpp>
#include <Rk/Modular.hpp>
#include <Rk/Mutex.hpp>
#include <Rk/File.hpp>

#include <algorithm>
#include <string>

namespace Co
{
  //
  // = TextureImpl =====================================================================================================
  //
  class TextureImpl :
    public Texture
  {
    std::string   path;
    bool          wrap,
                  min_filter,
                  mag_filter;
    TexImage::Ptr image;
    Rk::Mutex     mutex;

    virtual TexImage::Ptr retrieve ()
    {
      auto lock = mutex.get_lock ();
      return std::move (image);
    }
    
  public:
    typedef std::shared_ptr <TextureImpl> Ptr;

    TextureImpl (Rk::StringRef new_path, u8 flags) :
      path       (/*"Textures/" +*/ new_path.string ()),
      wrap       (bool (flags & texture_wrap)),
      min_filter (bool (flags & texture_minfilter)),
      mag_filter (bool (flags & texture_magfilter))
    { }
    
    void construct (Ptr& self, WorkQueue& queue, LoadContext& ctx)
    {
      using Rk::ChunkLoader;
      
      //Rk::File file (path, Rk::File::open_read_existing);
      auto file = ctx.fs.open_read (path);

      char magic [8];
      Rk::get (*file, magic);
      if (Rk::StringRef (magic, 8) != "COTEXTR1")
        throw std::runtime_error ("Source file corrupt or not a texture");

      TextureHeader header = { 0 };
      u32 width, height;
      std::vector <u8> buffer;
      
      auto loader = Rk::make_chunk_loader (*file);

      while (loader.resume ())
      {
        switch (loader.type)
        {
          case chunk_type ('H', 'E', 'A', 'D'):
            if (header.version)
              throw std::runtime_error ("Texture has more than one HEAD");
            file -> read (&header, std::min (uptr (loader.size), sizeof (header)));
            if (header.version != 0x20120711)
              throw std::runtime_error ("Texture is of an unsupported version");
            if (header.flags & ~u8 (texflag_mask_))
              throw std::runtime_error ("Texture contains invalid flags");
            if (header.map_count == 0)
              throw std::runtime_error ("Texture contains no maps");
            if (header.map_count > 15)
              throw std::runtime_error ("Texture contains too many maps");
            width  = header.width;  if (width  == 0) width  = 0x10000;
            height = header.height; if (height == 0) height = 0x10000;
            if ((header.flags & texflag_cube_map) && (width != height))
              throw std::runtime_error ("Texture contains rectangular cubemap faces");
            if (header.format >= texformat_count)
              throw std::runtime_error ("Texture uses invalid image format");
          break;
          
          case chunk_type ('D', 'A', 'T', 'A'):
            if (!header.version)
              throw std::runtime_error ("Texture has DATA before HEAD");
            buffer.resize (loader.size);
            file -> read (buffer.data (), loader.size);
          break;
          
          default:
            file -> seek (loader.size);
        }
      }

      if (!header.version)
        throw std::runtime_error ("Texture has no chunks");

      if (buffer.empty ())
        throw std::runtime_error ("Texture has no DATA");
      
      /*TexImageFilter filter_type;

      if (filter)
        filter_type = texfilter_linear;
      else
        filter_type = texfilter_none;

      if (filter && header.type != textype_cube && header.map_count > 1)
        filter_type = texfilter_trilinear;*/

      /*if (header.flags & texflag_cube_map)
      {
        
      }*/

      TexImage::Ptr new_image;
      
      if (header.flags & texflag_cube_map) // cube map
      {
        if (header.layer_count != 0)
          throw std::runtime_error ("Texture is a cube map, but has multiple layers");

        new_image = ctx.rc.create_tex_cube (
          queue,
          (TexFormat) header.format,
          width,
          height,
          min_filter,
          mag_filter
        );

        uptr offset = 0;

        for (uint face = 0; face != 6; face++)
        {
          for (uint level = 0; level != header.map_count; level++)
            offset += new_image -> load_map (face, level, buffer.data () + offset, buffer.size () - offset);
        }
      }
      else if (header.layer_count == 0) // 2d
      {
        new_image = ctx.rc.create_tex_image_2d (
          queue,
          (TexFormat) header.format,
          width,
          height,
          header.map_count,
          wrap ? texwrap_wrap : texwrap_clamp,
          min_filter,
          mag_filter,
          buffer.data (),
          buffer.size ()
        );
      }
      else // array
      {
        new_image = ctx.rc.create_tex_array (
          queue,
          (TexFormat) header.format,
          width,
          height,
          header.layer_count,
          header.map_count,
          wrap ? texwrap_wrap : texwrap_clamp,
          min_filter,
          mag_filter
        );

        uptr offset = 0;

        for (uint layer = 0; layer != header.layer_count; layer++)
        {
          for (uint level = 0; level != header.map_count; level++)
            offset += new_image -> load_map (layer, level, buffer.data () + offset, buffer.size () - offset);
        }
      }

      auto lock = mutex.get_lock ();
      image = std::move (new_image);
    }

  }; // class TextureImpl

  //
  // = FactoryImpl =====================================================================================================
  //
  class FactoryImpl :
    public TextureFactory,
    public ResourceFactory <std::string, TextureImpl>
  {
    Log&       log;
    WorkQueue& queue;

    virtual Texture::Ptr create (
      Rk::StringRef path,
      u8            flags)
    {
      auto ptr = find (path.string ());
      if (!ptr)
      {
        ptr = queue.gc_construct (new TextureImpl (path, flags));
        add (path.string (), ptr);
      }
      return std::move (ptr);
    }

  public:
    FactoryImpl (Log& log, WorkQueue& queue) :
      log   (log),
      queue (queue)
    { }
    
  };

  class Root :
    public TextureRoot
  {
    virtual TextureFactory::Ptr create_factory (Log& log, WorkQueue& queue)
    {
      return std::make_shared <FactoryImpl> (log, queue);
    }

  };

  RK_MODULE (Root);

} // namespace Co