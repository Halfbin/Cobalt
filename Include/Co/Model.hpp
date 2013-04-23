//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_MODEL
#define CO_H_MODEL

// Uses
#include <Co/GeomCompilation.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/Frame.hpp>

#include <Rk/StringRef.hpp>

#include <memory>

namespace Co
{
  class Model
  {
    GeomCompilation::Ptr comp;
    const Mesh*          meshes;
    uint                 mesh_count;

    virtual GeomCompilation::Ptr retrieve (const Mesh*& meshes, uint& mesh_count) = 0;

  public:
    typedef std::shared_ptr <Model> Ptr;

    bool ready ()
    {
      if (!comp)
        comp = retrieve (meshes, mesh_count);
      return comp;
    }

    void draw (Frame& frame, Spatial spat, const Material* materials, uint material_count)
    {
      if (!ready ())
        return;

      frame.begin_point_geom (comp, spat);
        frame.add_meshes    (meshes,    mesh_count);
        frame.add_materials (materials, material_count);
      frame.end ();
    }

  }; // class Model

  class ModelFactory
  {
  public:
    typedef std::shared_ptr <ModelFactory> Ptr;

    virtual Model::Ptr create (Rk::StringRef path) = 0;

  };

  class ModelRoot
  {
  public:
    virtual ModelFactory::Ptr create_factory (Log& log, WorkQueue& queue) = 0;

  };

} // namespace Co

#endif
