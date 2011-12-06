//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxTextureFactory.hpp>
#include <Co/IxTexture.hpp>

// Uses
#include <Co/IxRenderContext.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/TextureFile.hpp>

#include <Rk/ChunkLoader.hpp>
#include <Rk/ShortString.hpp>
#include <Rk/Expose.hpp>
#include <Rk/MinMax.hpp>
#include <Rk/File.hpp>

#include <unordered_map>

namespace Co
{
  //
  // = Texture =========================================================================================================
  //
  class Texture :
    public IxTexture
  {
    Rk::ShortString <512> path;
    bool                  wrap,
                          filter;

    long ref_count;

    virtual void acquire ()
    {
      _InterlockedIncrement (&ref_count);
    }

    virtual void release ()
    {
      _InterlockedDecrement (&ref_count);
    }

    virtual void dispose ()
    {
      delete this;
    }

    ~Texture ()
    {
      image -> destroy ();
    }

    virtual void load (IxRenderContext& rc)
    {
      using Rk::ChunkLoader;
      
      Rk::File file (path, Rk::File::open_read_existing);

      char magic [8];
      file.get_array (magic);
      if (Rk::StringRef (magic, 8) != "COTEXTR1")
        throw Rk::Exception ("Source file corrupt or not a texture");

      TextureHeader header = { 0 };
      u32 width, height;
      std::unique_ptr <u8 []> buffer;
      
      ChunkLoader loader (file);

      while (loader.resume ())
      {
        switch (loader.type)
        {
          case chunk_type ('H', 'E', 'A', 'D'):
            if (header.version)
              throw Rk::Exception ("Texture has more than one HEAD");
            file.read (&header, Rk::minimum (loader.size, sizeof (header)));
            if (header.version != 0x20111206)
              throw Rk::Exception ("Texture is of an unsupported version");
            if (header.flags & ~u8 (texflag_mask))
              throw Rk::Exception ("Texture contains invalid flags");
            if (header.map_count == 0)
              throw Rk::Exception ("Texture contains no maps");
            if (header.map_count > 15)
              throw Rk::Exception ("Texture contains too many maps");
            width  = header.width;  if (width  == 0) width  = 0x10000;
            height = header.height; if (height == 0) height = 0x10000;
            if ((header.flags & texflag_cube_map) && (width != height))
              throw Rk::Exception ("Texture contains rectangular cubemap faces");
            if (header.format >= texformat_count)
              throw Rk::Exception ("Texture uses invalid image format");
          break;
          
          case chunk_type ('D', 'A', 'T', 'A'):
            if (!header.version)
              throw Rk::Exception ("Texture has DATA before HEAD");
            buffer.reset (new u8 [loader.size]);
            file.read (buffer.get (), loader.size);
          break;
          
          default:
            file.seek (loader.size);
        }
      }

      if (!header.version)
        throw Rk::Exception ("Texture has no chunks");

      if (!buffer)
        throw Rk::Exception ("Texture has no DATA");
      
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

      image = rc.create_tex_image (header.map_count, wrap ? texwrap_wrap : texwrap_clamp, filter, type);

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

          image -> load_map (sub_image, level, buffer.get () + offset, (TexFormat) header.format, w, h, size);
          offset += size;
          w /= 2;
          h /= 2;
        }
      }

      ready = true;
    }

  public:
    Texture (IxLoadContext* context, Rk::StringRef new_path, bool new_wrap, bool new_filter)
    {
      ref_count = 1;
      path = context -> get_game_path ();
      path += "Textures/";
      path += new_path;
      wrap = new_wrap;
      filter = new_filter;
      image = 0;
      context -> load (this);
    }

    bool dead () const
    {
      return ref_count == 0;
    }

  }; // class Texture

  //
  // = TextureFactory ==================================================================================================
  //
  class TextureFactory :
    public IxTextureFactory
  {
    typedef std::unordered_map <Rk::ShortString <512>, IxTexture*> CacheType;
    CacheType cache;

    virtual IxTexture* create (IxLoadContext* context, Rk::StringRef path, bool wrap, bool filter)
    {
      IxTexture* tex;

      auto iter = cache.find (path);

      if (iter != cache.end ())
      {
        tex = iter -> second;
        tex -> acquire ();
      }
      else
      {
        tex = new Texture (context, path, wrap, filter);
        cache.insert (CacheType::value_type (path, tex));
      }
      
      return tex;
    }

  } factory;

  IX_EXPOSE (void** out, u64 ixid)
  {
    Rk::expose <IxTextureFactory> (factory, ixid, out);
  }

} // namespace Co