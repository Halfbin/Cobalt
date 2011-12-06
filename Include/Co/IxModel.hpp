//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXMODEL
#define CO_H_IXMODEL

// Implements
#include <Co/IxResource.hpp>

// Uses
#include <Co/IxFrame.hpp>

namespace Co
{
  class IxModel : 
    public IxResource
  {
  protected:
    IxGeomCompilation* comp;
    Mesh*              meshes;
    uint               mesh_count;

  public:
    typedef Rk::IxSharedPtr <IxModel> Ptr;

    void draw (IxFrame* target, Spatial prev, Spatial cur, const Material* materials, uint material_count)
    {
      if (!ready)
        return;

      target -> begin_point_geom (comp, prev, cur);
        target -> add_meshes    (meshes,    mesh_count);
        target -> add_materials (materials, material_count);
      target -> end_point_geom ();
    }

  }; // class IxModel

} // namespace Co

#endif
