//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_FRAME
#define CO_H_FRAME

#include <Rk/Types.hpp>

#include <Co/GeomCompilation.hpp>
#include <Co/GeomBuffer.hpp>
#include <Co/TexImage.hpp>
#include <Co/Spatial.hpp>

namespace Co
{
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
    v4f           ambient_mat,
                  diffuse_mat,
                  specular_mat,
                  emissive_mat;
    TexImage::Ptr diffuse_tex,
                  specular_tex,
                  emissive_tex,
                  exponent_tex,
                  normal_tex,
                  self_illum_tex;

    Material ()
    { }

    Material (Nil) :
      ambient_mat    (nil),
      diffuse_mat    (nil),
      specular_mat   (nil),
      emissive_mat   (nil)
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
    v4f location,
        diffuse,
        specular;
  };

  //
  // = Frame ===========================================================================================================
  //
  class Frame
  {
  public:
    virtual void begin_point_geom (GeomCompilation::Ptr compilation, Spatial spat) = 0;
    virtual void end              () = 0;

    virtual void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      const TexRect* end,
      Spatial2D      spat,
      v4f            linear_colour,
      v4f            const_colour
    ) = 0;

    virtual void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      const TexRect* end,
      Spatial        spat,
      v4f            linear_colour,
      v4f            const_colour
    ) = 0;

    virtual void add_lights (const Light* lights, const Light* end) = 0;
    virtual void set_skybox (TexImage::Ptr cube, v3f colour, float alpha) = 0;

    virtual void set_camera (Spatial spat, float fov, float near, float far) = 0;
    
    virtual void add_meshes    (const Mesh*     begin, const Mesh*     end) = 0;
    virtual void add_materials (const Material* begin, const Material* end) = 0;

    void add_meshes (const Mesh* begin, uint count)
    {
      add_meshes (begin, begin + count);
    }

    void add_materials (const Material* begin, uint count)
    {
      add_materials (begin, begin + count);
    }

    void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      uint           count,
      Spatial2D      spat,
      v4f            linear_colour,
      v4f            const_colour)
    {
      add_label (texture, rects, rects + count, spat, linear_colour, const_colour);
    }

    void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      uint           count,
      Spatial        spat,
      v4f            linear_colour,
      v4f            const_colour)
    {
      add_label (texture, rects, rects + count, spat, linear_colour, const_colour);
    }

    void add_lights (const Light* begin, uint count)
    {
      add_lights (begin, begin + count);
    }

  }; // Frame

} // namespace Co

#endif
