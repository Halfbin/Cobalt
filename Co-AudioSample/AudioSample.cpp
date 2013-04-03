//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/AudioSample.hpp>

// Uses
#include <Co/ResourceFactory.hpp>
#include <Co/AudioService.hpp>
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
  // = SampleImpl ======================================================================================================
  //
  class SampleImpl :
    public AudioSample
  {
    std::string      path;
    AudioBuffer::Ptr buffer;
    AudioFormat      format;
    Rk::Mutex        mutex;

    virtual AudioBuffer::Ptr retrieve (AudioFormat& fmt)
    {
      auto lock = mutex.get_lock ();
      fmt = format;
      return std::move (buffer);
    }
    
  public:
    typedef std::shared_ptr <SampleImpl> Ptr;

    SampleImpl (Rk::StringRef new_path) :
      path (new_path.string ())
    { }
    
    void construct (Ptr& self, WorkQueue& queue, LoadContext& ctx)
    {
      using Rk::ChunkLoader;
      
      AudioFormat new_format (audio_coding_pcm, audio_channels_mono, 44100, 16);

      static const uint samples = 44100;
      i16 data [samples];

      for (uint i = 0; i != samples; i++)
        data [i] = i16 (
          32760.f *
          0.25f *
          std::sin (float (i) / 44100.0f * 2.0f * 3.14159f * 880.0f) *
          std::sin (float (i) / 44100.0f * 2.0f * 3.14159f * 0.5f)
        );

      AudioBuffer::Ptr new_buffer = ctx.as.create_buffer (new_format, data, samples * 2, samples);

      //Rk::File file (path, Rk::File::open_read_existing);
      /*auto file = fs.open_read (path);

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
      

      TexImage::Ptr new_image;
      
      if (header.flags & texflag_cube_map) // cube map
      {
        if (header.layer_count != 0)
          throw std::runtime_error ("Texture is a cube map, but has multiple layers");

        new_image = rc.create_tex_cube (
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
        new_image = rc.create_tex_image_2d (
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
        new_image = rc.create_tex_array (
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
      }*/
      
      auto lock = mutex.get_lock ();
      buffer = std::move (new_buffer);
      format = new_format;
    }

  }; // class SampleImpl

  //
  // = FactoryImpl =====================================================================================================
  //
  class FactoryImpl :
    public AudioSampleFactory,
    public ResourceFactory <std::string, SampleImpl>
  {
    Log&       log;
    WorkQueue& queue;

    virtual AudioSample::Ptr create (Rk::StringRef path)
    {
      auto ptr = find (path.string ());
      if (!ptr)
      {
        ptr = queue.gc_construct (new SampleImpl (path));
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
    public AudioSampleRoot
  {
    virtual AudioSampleFactory::Ptr create_factory (Log& log, WorkQueue& queue)
    {
      return std::make_shared <FactoryImpl> (log, queue);
    }

  };

  RK_MODULE (Root);

} // namespace Co
