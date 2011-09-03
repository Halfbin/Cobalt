//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxTextureFactory.hpp>

// Uses
#include <Co/IxRenderContext.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/TextureFile.hpp>
#include <Co/IxTexture.hpp>

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

      TextureHeader header = { 0, 0, 0, 0, 0 };
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
            if (header.version != 0x20110829)
              throw Rk::Exception ("Texture is of an unsupported version");
            if (header.map_count == 0)
              throw Rk::Exception ("Texture contains no maps");
            if (header.map_count > 15)
              throw Rk::Exception ("Texture contains too many mipmaps");
            width  = header.width;  if (width  == 0) width  = 0x10000;
            height = header.height; if (height == 0) height = 0x10000;
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
      
      image = rc.create_tex_image (header.map_count, false);

      u32 offset = 0;
      u32 size = header.width / 4 * header.height / 4 * 8;

      for (uint level = 0; level != header.map_count; level++)
      {
        image -> load_map (level, buffer.get () + offset, (TexFormat) header.format, header.width, header.height, size);
        offset += size;
        size          >>= 2;
        header.width  >>= 1;
        header.height >>= 1;
      }

      ready = true;
    }

  public:
    Texture (IxLoadContext& context, Rk::StringRef new_path)
    {
      ref_count = 1;
      path = context.get_game_path ();
      path += "Textures/";
      path += new_path;
      image = 0;
      context.load (this);
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

    virtual IxTexture* create (IxLoadContext& context, Rk::StringRef path)
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
        tex = new Texture (context, path);
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