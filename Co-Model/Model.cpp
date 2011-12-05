//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxModelFactory.hpp>
#include <Co/IxModel.hpp>

// Uses
#include <Co/IxGeomCompilation.hpp>
#include <Co/IxRenderContext.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/IxGeomBuffer.hpp>

#include <Rk/ChunkLoader.hpp>
#include <Rk/ShortString.hpp>
#include <Rk/Expose.hpp>
#include <Rk/File.hpp>

#include <unordered_map>

namespace
{
  extern "C" long _InterlockedIncrement (long*);
  #pragma intrinsic (_InterlockedIncrement)

  extern "C" long _InterlockedDecrement (long*);
  #pragma intrinsic (_InterlockedDecrement)

}

namespace Co
{
  //
  // = Model ===========================================================================================================
  //
  class Model :
    public IxModel
  {
    struct Header
    {
      u32 version,
          index_count,
          element_count;
      u16 material_count,
          vismesh_count;
    };
    static_assert (sizeof (Header) == 16, "Model::Header miscompiled");

    struct VisMesh
    {
      u16 first_index,
          element_count,
          material;
    };
    static_assert (sizeof (VisMesh) == 6, "Model::VisMesh miscompiled");

    Co::IxGeomBuffer::Ptr element_buffer,
                          index_buffer;
    Rk::ShortString <512> path;
    long                  ref_count;

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

    ~Model ()
    {
      comp -> destroy ();
      delete [] meshes;
    }
    
    virtual void load (IxRenderContext& rc) try
    {
      using Rk::ChunkLoader;
      
      Rk::File file (path, Rk::File::open_read_existing);

      char magic [8];
      file.get_array (magic);
      if (Rk::StringRef (magic, 8) != "RKMODEL1")
        throw Rk::Exception ("Source file corrupt or not a model");

      ChunkLoader loader (file);
      Header header;

      std::unique_ptr <u8 []> buffer;
      uint buffer_size = 0;
      auto buffer_reserve = [&buffer, &buffer_size] (uint size)
      {
        if (size <= buffer_size) return;
        buffer.reset (new u8 [size]);
        buffer_size = size;
      };

      while (loader.resume ())
      {
        switch (loader.type)
        {
          case chunk_type ('H', 'E', 'A', 'D'):
            file.read (&header, loader.size);
            if (header.version != (2011 << 16 | 3 << 8 | 27))
              throw Rk::Exception ("Model is of an unsupported version");
            element_buffer = rc.create_buffer (header.element_count * 32);
            index_buffer   = rc.create_buffer (header.index_count   *  2);
            mesh_count = header.vismesh_count;
            meshes = new Mesh [mesh_count];
          break;
          
          case chunk_type ('E', 'L', 'E', 'M'):
            buffer_reserve (loader.size);
            file.read (buffer.get (), loader.size);
            element_buffer -> load_data (buffer.get (), loader.size);
          break;
          
          case chunk_type ('I', 'D', 'C', 'S'):
            buffer_reserve (loader.size);
            file.read (buffer.get (), loader.size);
            index_buffer -> load_data (buffer.get (), loader.size);
          break;
          
          case chunk_type ('V', 'M', 'S', 'H'):
          {
            buffer_reserve (loader.size);
            file.read (buffer.get (), loader.size);
            auto vis_meshes = (const VisMesh*) buffer.get ();
            for (uint i = 0; i != mesh_count; i++)
            {
              meshes [i].prim_type     = prim_triangles;
              meshes [i].material      = vis_meshes [i].material;
              meshes [i].base_element  = 0;
              meshes [i].base_index    = vis_meshes [i].first_index;
              meshes [i].element_count = vis_meshes [i].element_count;
            }
          }
          break;
          
          default:
            file.seek (loader.size);
        }
      }

      static GeomAttrib attribs [3] = {
        { attrib_position, attrib_f32, 32, 0  },
        { attrib_tcoords,  attrib_f32, 32, 12 },
        { attrib_normal,   attrib_f32, 32, 20 }
      };

      comp = rc.create_compilation (attribs, element_buffer, index_buffer, index_u16);

      ready = true;
    }
    catch (...)
    {
      delete [] meshes;
      throw;
    }

  public:
    Model (IxLoadContext& context, Rk::StringRef new_path)
    {
      ref_count = 1;
      path = context.get_game_path ();
      path += "Models/";
      path += new_path;
      comp = 0;
      meshes = 0;
      mesh_count = 0;
      context.load (this);
    }

    bool dead () const
    {
      return ref_count == 0;
    }

  }; // class Model

  //
  // = ModelFactory ====================================================================================================
  //
  class ModelFactory :
    public IxModelFactory
  {
    typedef std::unordered_map <Rk::ShortString <512>, IxModel*> CacheType;
    CacheType cache;

    virtual IxModel* create (IxLoadContext& context, Rk::StringRef path)
    {
      IxModel* model;

      auto iter = cache.find (path);

      if (iter != cache.end ())
      {
        model = iter -> second;
        model -> acquire ();
      }
      else
      {
        model = new Model (context, path);
        cache.insert (CacheType::value_type (path, model));
      }
      
      return model;
    }

  } factory;

  IX_EXPOSE (void** out, u64 ixid)
  {
    Rk::expose <IxModelFactory> (factory, ixid, out);
  }

} // namespace Co
