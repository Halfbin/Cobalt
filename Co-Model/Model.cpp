//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Model.hpp>

// Uses
#include <Co/GeomCompilation.hpp>
#include <Co/ResourceFactory.hpp>
#include <Co/RenderContext.hpp>
#include <Co/Filesystem.hpp>
#include <Co/GeomBuffer.hpp>
#include <Co/WorkQueue.hpp>

#include <Rk/AsyncMethod.hpp>
#include <Rk/ChunkLoader.hpp>
#include <Rk/Module.hpp>
#include <Rk/Mutex.hpp>
#include <Rk/File.hpp>

#include <unordered_map>
#include <vector>

namespace Co
{
  //
  // = ModelImpl =======================================================================================================
  //
  class ModelImpl :
    public Model
  {
    struct Header
    {
      u32 version,
          index_count,
          element_count;
      u16 material_count,
          vismesh_count;
    };
    static_assert (sizeof (Header) == 16, "ModelImpl::Header miscompiled");

    struct VisMesh
    {
      u16 first_index,
          element_count,
          material;
    };
    static_assert (sizeof (VisMesh) == 6, "ModelImpl::VisMesh miscompiled");

    GeomCompilation::Ptr comp;
    std::string          path;
    std::vector <Mesh>   meshes;
    Rk::Mutex            mutex;

    virtual GeomCompilation::Ptr retrieve (const Mesh*& meshes_out, uint& count_out)
    {
      meshes_out = meshes.data ();
      count_out  = meshes.size ();
      return std::move (comp);
    }

    ModelImpl (Rk::StringRef new_path) :
      path ("Models/" + new_path.string ())
    { }
    
  public:
    void construct (std::shared_ptr <ModelImpl>& self, WorkQueue& queue, RenderContext& rc, Filesystem& fs)
    {
      using Rk::ChunkLoader;
      
      auto file = fs.open_read (path);

      char magic [8];
      Rk::get (*file, magic);
      if (Rk::StringRef (magic, 8) != "RKMODEL1")
        throw std::runtime_error ("Source file corrupt or not a model");

      auto loader = Rk::make_chunk_loader (*file);
      Header header;

      std::unique_ptr <u8 []> buffer;
      uint buffer_size = 0;
      auto buffer_reserve = [&buffer, &buffer_size] (uint size)
      {
        if (size <= buffer_size) return;
        buffer.reset (new u8 [size]);
        buffer_size = size;
      };

      GeomBuffer::Ptr element_buffer,
                      index_buffer;

      while (loader.resume ())
      {
        switch (loader.type)
        {
          case chunk_type ('H', 'E', 'A', 'D'):
            file -> read (&header, loader.size);
            if (header.version != (2011 << 16 | 3 << 8 | 27))
              throw std::runtime_error ("Model is of an unsupported version");
            element_buffer = rc.create_buffer (queue, header.element_count * 32);
            index_buffer   = rc.create_buffer (queue, header.index_count   *  2);
            meshes.resize (header.vismesh_count);
          break;
          
          case chunk_type ('E', 'L', 'E', 'M'):
            buffer_reserve (loader.size);
            file -> read (buffer.get (), loader.size);
            element_buffer -> load_data (buffer.get (), loader.size);
          break;
          
          case chunk_type ('I', 'D', 'C', 'S'):
            buffer_reserve (loader.size);
            file -> read (buffer.get (), loader.size);
            index_buffer -> load_data (buffer.get (), loader.size);
          break;
          
          case chunk_type ('V', 'M', 'S', 'H'):
          {
            buffer_reserve (loader.size);
            file -> read (buffer.get (), loader.size);
            auto vis_meshes = (const VisMesh*) buffer.get ();
            for (uint i = 0; i != meshes.size (); i++)
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
            file -> seek (loader.size);
        }
      }

      static GeomAttrib attribs [3] = {
        { attrib_position, attrib_f32, 32, 0  },
        { attrib_tcoords,  attrib_f32, 32, 12 },
        { attrib_normal,   attrib_f32, 32, 20 }
      };

      auto lock = mutex.get_lock ();
      comp = rc.create_compilation (queue, attribs, element_buffer, index_buffer, index_u16);
    }

    static Ptr create (WorkQueue& queue, Rk::StringRef new_path)
    {
      return queue.gc_construct (new ModelImpl (new_path));
    }

  };

  //
  // = FactoryImpl =====================================================================================================
  //
  class FactoryImpl :
    public ModelFactory,
    public ResourceFactory <std::string, ModelImpl>
  {
    virtual Model::Ptr create (WorkQueue& queue, Rk::StringRef path)
    {
      auto ptr = find (path.string ());
      if (!ptr)
      {
        ptr = ModelImpl::create (queue, path);
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
