//
// Copyright (C) 2011 Roadkill Software
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
#include <Rk/Module.hpp>
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
                  filter;
    TexImage::Ptr image;
    Rk::Mutex     mutex;

    virtual TexImage::Ptr retrieve ()
    {
      auto lock = mutex.get_lock ();
      return std::move (image);
    }

    TextureImpl (Rk::StringRef new_path, bool new_wrap, bool new_filter) :
      path   ("Textures/" + new_path.string ()),
      wrap   (new_wrap),
      filter (new_filter)
    { }
    
  public:
    typedef std::shared_ptr <TextureImpl> Ptr;

    static Ptr create (WorkQueue& queue, Rk::StringRef new_path, bool new_wrap, bool new_filter)
    {
      return queue.gc_construct (new TextureImpl (new_path, new_wrap, new_filter));
    }

    void construct (Ptr& self, WorkQueue& queue, RenderContext& rc, Filesystem& fs)
    {
      using Rk::ChunkLoader;
      
      //Rk::File file (path, Rk::File::open_read_existing);
      auto file = fs.open_read (path);

      char magic [8];
      Rk::get (*file, magic);
      if (Rk::StringRef (magic, 8) != "COTEXTR1")
        throw std::runtime_error ("Source file corrupt or not a texture");

      TextureHeader header = { 0 };
      u32 width, height;
      std::unique_ptr <u8 []> buffer;
      
      auto loader = Rk::make_chunk_loader (*file);

      while (loader.resume ())
      {
        switch (loader.type)
        {
          case chunk_type ('H', 'E', 'A', 'D'):
            if (header.version)
              throw std::runtime_error ("Texture has more than one HEAD");
            file -> read (&header, Rk::minimum (loader.size, sizeof (header)));
            if (header.version != 0x20111206)
              throw std::runtime_error ("Texture is of an unsupported version");
            if (header.flags & ~u8 (texflag_mask))
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
            buffer.reset (new u8 [loader.size]);
            file -> read (buffer.get (), loader.size);
          break;
          
          default:
            file -> seek (loader.size);
        }
      }

      if (!header.version)
        throw std::runtime_error ("Texture has no chunks");

      if (!buffer)
        throw std::runtime_error ("Texture has no DATA");
      
      /*TexImageFilter filter_type;

      if (filter)
        filter_type = texfilter_linear;
      else
        filter_type = texfilter_none;

      if (filter && header.type != textype_cube && header.map_count > 1)
        filter_type = texfilter_trilinear;*/

      TexImageType type = textype_2d;
      if (header.flags & texflag_cube_map)
        type = textype_cube;

      TexImage::Ptr new_image = rc.create_tex_image (queue, header.map_count, wrap ? texwrap_wrap : texwrap_clamp, filter, type);

      u32 offset = 0;
      
      uint sub_image_count = 1;
      if (header.flags & texflag_cube_map)
        sub_image_count = 6;

      for (uint sub_image = 0; sub_image != sub_image_count; sub_image++)
      {
        u32 w = width,
            h = height;

        for (uint level = 0; level != header.map_count; level++)
        {
          u32 size;

          if (header.format == texformat_dxt1)
            size = w / 4 * h / 4 * 8;
          /*else if (header.format == texformat_bgr888)
            size = Rk::align (header.width * 3, 4) * header.height;*/
          else if (header.format == texformat_rgba8888)
            size = w * h * 4;

          new_image -> load_map (sub_image, level, buffer.get () + offset, (TexFormat) header.format, w, h, size);
          offset += size;
          w /= 2;
          h /= 2;
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
    virtual Texture::Ptr create (WorkQueue& queue, Rk::StringRef path, bool wrap, bool filter)
    {
      auto ptr = find (path.string ());
      if (!ptr)
      {
        ptr = TextureImpl::create (queue, path, wrap, filter);
        add (path.string (), ptr);
      }
      return std::move (ptr);
    }

  public:
    static std::shared_ptr <FactoryImpl> create ()
    {
      return std::make_shared <FactoryImpl> ();
    }

  };

  RK_MODULE_FACTORY (FactoryImpl);

} // namespace Co