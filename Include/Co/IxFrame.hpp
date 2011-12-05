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
    IxTexImage *diffuse_tex,
               *specular_tex,
               *emissive_tex,
               *exponent_tex,
               *normal_tex,
               *self_illum_tex;

    Material () { }

    Material (const Nil&) :
      ambient_mat    (nil),
      diffuse_mat    (nil),
      specular_mat   (nil),
      emissive_mat   (nil),
      diffuse_tex    (0),
      specular_tex   (0),
      emissive_tex   (0),
      exponent_tex   (0),
      normal_tex     (0),
      self_illum_tex (0)
    { }
    
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
  class IxFrame
  {
  public:
    virtual bool begin_point_geom (IxGeomCompilation* compilation, Spatial prev, Spatial current) = 0;
    virtual void end_point_geom   () = 0;
    virtual void add_meshes       (const Mesh* meshes, const Mesh* end) = 0;
    virtual void add_materials    (const Material* materials, const Material* end) = 0;
    virtual void add_ui_batch     (IxTexImage* texture, const TexRect* rects, const TexRect* end) = 0;
    virtual void add_label        (IxTexImage* texture, Spatial prev, Spatial current, const TexRect* begin, const TexRect* end) = 0;
    virtual void add_lights       (const Light* lights, const Light* end) = 0;
    virtual void set_camera       (Spatial prev, Spatial current, float prev_fov, float current_fov, float near, float far) = 0;
    virtual void set_size         (u32 width, u32 height) = 0;

    void add_meshes (const Mesh* meshes, uint count)
    {
      add_meshes (meshes, meshes + count);
    }

    void add_materials (const Material* materials, uint count)
    {
      add_materials (materials, materials + count);
    }

    void add_ui_batch (IxTexImage* texture, const TexRect* rects, uint count)
    {
      add_ui_batch (texture, rects, rects + count);
    }

    void add_label (IxTexImage* texture, Spatial prev, Spatial current, const TexRect* begin, uint count)
    {
      add_label (texture, prev, current, begin, begin + count);
    }

    void add_lights (const Light* lights, uint count)
    {
      add_lights (lights, lights + count);
    }

  };

} // namespace Co

#endif
