//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXFRAME
#define CO_H_IXFRAME

#include <Rk/Types.hpp>
#include <Co/Spatial.hpp>

namespace Co
{
  class IxGeomCompilation;
  class IxTexImage;
  
  //
  // = Meshes ==========================================================================================================
  //
  enum PrimType :
    u16
  {
    prim_points = 0,
    prim_lines,
    prim_line_loop,
    prim_line_strip,
    prim_triangles,
    prim_triangle_strip,
    prim_triangle_fan
  };
  
  struct Mesh
  {
    PrimType prim_type;     // 2
    u16      material;      // 4
    u32      base_element,  // 8
             base_index,    // 12
             element_count; // 16

    Mesh () { }

    Mesh (PrimType new_prim_type, u16 new_material, u32 new_base_element, u32 new_base_index, u32 new_element_count) :
      prim_type     (new_prim_type),
      material      (new_material),
      base_element  (new_base_element),
      base_index    (new_base_index),
      element_count (new_element_count)
    { }
    
  };

  //
  // = Material ========================================================================================================
  //
  struct Material
  {
    Vector4    ambient_mat,
               diffuse_mat,
               specular_mat,
               emissive_mat;
    float      exponent_mat;
    IxTexImage *diffuse_tex,
               *specular_tex,
               *emissive_tex,
               *exponent_tex,
               *normal_tex;
  };

  //
  // = TexRect =========================================================================================================
  //
  struct TexRect
  {
    i32 x,  y,
        w,  h,
        s,  t,
        tw, th;

    TexRect (const Nil& n = nil) :
      x (0), y (0), w  (0), h  (0),
      s (0), t (0), tw (0), th (0)
    { }
    
    TexRect (i32 x, i32 y, i32 w, i32 h, i32 s, i32 t, i32 tw, i32 th) :
      x (x), y (y), w  (w),  h  (h),
      s (s), t (t), tw (tw), th (th)
    { }
    
  };

  //
  // = Light ===========================================================================================================
  //
  struct Light
  {
    Vector4 location,
            diffuse,
            specular;
  };

  //
  // = IxFrame =========================================================================================================
  //
  class IxFrame :
    Rk::NoCopy
  {
  protected:
    struct PointSpatial
    {
      Spatial prev, // 32
              cur;  // 64
    };
    
    struct PointGeom
    {
      u32 mesh_count,     // 4
          material_count; // 8
    };

    struct UIBatch
    {
      IxTexImage* tex;   //  4  8
      u32         first, //  8 12
                  count; // 12 16
    };

    struct Label
    {
      IxTexImage* tex;   //  4  8
      u32         first, //  8 12
                  count; // 12 16
    };

    // Limits
    const u32 max_point_geoms,
              max_meshes,
              max_materials,
              max_ui_batches,
              max_labels,
              max_tex_rects,
              max_lights;

    // Indices
    u32 point_geoms_back_index,
        meshes_back_index,
        materials_back_index,
        ui_batches_back_index,
        labels_back_index,
        tex_rects_back_index,
        lights_back_index;

    // Drawing data
    PointSpatial*       const point_spats;
    PointGeom*          const point_geoms;
    IxGeomCompilation** const point_comps;
    Mesh*               const meshes;
    Material*           const materials;
    UIBatch*            const ui_batches;
    Label*              const labels;
    PointSpatial*       const label_spats;
    TexRect*            const tex_rects;
    Light*              const lights;

    // Camera
    PointSpatial camera_pos;
    float        camera_prev_fov,
                 camera_cur_fov,
                 camera_near,
                 camera_far;

    // Viewport
    uint width,
         height;

  protected:
    IxFrame (
      PointSpatial*       new_point_spats,
      PointGeom*          new_point_geoms,
      IxGeomCompilation** new_point_comps,
      Mesh*               new_meshes,
      Material*           new_materials,
      UIBatch*            new_ui_batches,
      Label*              new_labels,
      PointSpatial*       new_label_spats,
      TexRect*            new_tex_rects,
      Light*              new_lights,
      u32                 new_max_point_geoms,
      u32                 new_max_meshes,
      u32                 new_max_materials,
      u32                 new_max_ui_batches,
      u32                 new_max_labels,
      u32                 new_max_tex_rects,
      u32                 new_max_lights
    ) :
      point_spats     (new_point_spats),
      point_geoms     (new_point_geoms),
      point_comps     (new_point_comps),
      meshes          (new_meshes),
      materials       (new_materials),
      ui_batches      (new_ui_batches),
      labels          (new_labels),
      label_spats     (new_label_spats),
      tex_rects       (new_tex_rects),
      lights          (new_lights),
      max_point_geoms (new_max_point_geoms),
      max_meshes      (new_max_meshes),
      max_materials   (new_max_materials),
      max_ui_batches  (new_max_ui_batches),
      max_labels      (new_max_labels),
      max_tex_rects   (new_max_tex_rects),
      max_lights      (new_max_lights)
    { }
    
  public:
    bool begin_point_geom (IxGeomCompilation* comp, Spatial prev, Spatial cur)
    {
      if (point_geoms_back_index == max_point_geoms)
        return false;

      PointSpatial spat = { prev, cur };
      point_spats [point_geoms_back_index] = spat;
      
      PointGeom geom = { 0, 0 };
      point_geoms [point_geoms_back_index] = geom;
      
      point_comps [point_geoms_back_index] = comp;

      return true;
    }

    void end_point_geom ()
    {
      point_geoms_back_index++;
    }
    
    template <typename Iter>
    void add_meshes (Iter in, Iter end)
    {
      uint old_meshes_back_index = meshes_back_index;

      while (meshes_back_index != max_meshes && in != end)
        meshes [meshes_back_index++] = *in++;
      
      point_geoms [point_geoms_back_index].mesh_count += meshes_back_index - old_meshes_back_index;
    }
    
    template <typename Iter>
    void add_meshes (Iter in, uint count)
    {
      add_meshes (in, in + count);
    }

    template <typename Iter>
    void add_materials (Iter in, Iter end)
    {
      uint old_materials_back_index = materials_back_index;

      while (materials_back_index != max_materials && in != end)
        materials [materials_back_index++] = *in++;
      
      point_geoms [point_geoms_back_index].material_count += materials_back_index - old_materials_back_index;
    }
    
    template <typename Iter>
    void add_materials (Iter in, uint count)
    {
      add_materials (in, in + count);
    }

    template <typename Iter>
    void add_ui_batch (IxTexImage* tex, Iter begin, Iter end)
    {
      if (ui_batches_back_index == max_ui_batches)
        return;

      auto& batch = ui_batches [ui_batches_back_index];
      batch.tex = tex;
      batch.first = tex_rects_back_index;

      while (tex_rects_back_index != max_tex_rects && begin != end)
        tex_rects [tex_rects_back_index++] = *begin++;

      batch.count = tex_rects_back_index - batch.first;

      ui_batches_back_index++;
    }

    template <typename Iter>
    void add_ui_batch (IxTexImage* tex, Iter begin, uint count)
    {
      add_ui_batch (tex, x, y, begin, begin + count);
    }

    template <typename Iter>
    void add_label (IxTexImage* tex, Spatial prev, Spatial cur, Iter begin, Iter end)
    {
      if (labels_back_index == max_labels)
        return;

      auto& label = labels [labels_back_index];
      label.pack = pack;
      label.x = x;
      label.y = y;
      label.first = tex_rects_back_index;

      while (tex_rects_back_index != max_tex_rects && begin != end)
        tex_rects [tex_rects_back_index++] = *begin++;

      label.count = tex_rects_back_index - label.first;

      label_spats [labels_back_index].prev = prev;
      label_spats [labels_back_index].cur  = cur;
      
      labels_back_index++;
    }

    template <typename Iter>
    void add_label (IxTexImage* tex, Spatial prev, Spatial cur, Iter begin, uint count)
    {
      add_label (tex, prev, cur, begin, begin + count);
    }

    template <typename Iter>
    void add_lights (Iter begin, Iter end)
    {
      while (lights_back_index != max_lights && begin != end)
        lights [lights_back_index++] = *begin++;
    }
    
    void set_camera (Spatial prev, Spatial cur, float prev_fov, float cur_fov, float near, float far)
    {
      camera_pos.prev = prev;
      camera_pos.cur  = cur;
      camera_prev_fov = prev_fov;
      camera_cur_fov  = cur_fov;
      camera_near     = near;
      camera_far      = far;
    }
    
    void set_size (uint w, uint h)
    {
      width  = w;
      height = h;
    }

  }; // class IxFrame

} // namespace Co

#endif
