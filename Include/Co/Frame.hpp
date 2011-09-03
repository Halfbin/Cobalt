//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_FRAME
#define CO_H_FRAME

#include <Rk/Types.hpp>
#include <Co/Spatial.hpp>
#include <Rk/Vector4.hpp>

namespace Co
{
  class IxGeomCompilation;
  class IxTexImage;

  //
  // = Meshes ==========================================================================================================
  //
  enum PrimType :
    u32
  {
    prim_points = 0,
    prim_lines,
    prim_line_loop,
    prim_line_strip,
    prim_triangles,
    prim_triangle_strip,
    prim_triangle_fan
  };
  
  enum IndexType :
    u16
  {
    index_none = 0,
    index_u8,
    index_u16,
    index_u32
  };
  
  struct Mesh
  {
    u32 prim_type,     // 4
        first_item,    // 8
        element_count; // 12
    u16 index_type,    // 14
        material;      // 16
  };

  //
  // = Material ========================================================================================================
  //
  struct Material
  {
    Rk::Vector4f ambient_mat,
                 diffuse_mat,
                 specular_mat,
                 emissive_mat;
    float        exponent_mat;
    IxTexImage  *diffuse_tex,
                *specular_tex,
                *emissive_tex,
                *exponent_tex,
                *normal_tex;
  };

  //
  // = Frame ===========================================================================================================
  //
  class Frame
  {
  protected:
    enum Maxima
    {
      max_point_geoms = 256,
      max_meshes      = 256,
      max_ui_rects    = 256,
      max_lights      = 8,
      max_materials   = 1024
    };
    
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

    PointSpatial       point_spats [max_point_geoms];
    PointGeom          point_geoms [max_point_geoms];
    IxGeomCompilation* point_comps [max_point_geoms];
		
    Mesh     meshes    [max_meshes];
    Material materials [max_materials];
    //UIRect   ui_rects  [max_ui_rects];
    //Light    lights    [max_lights];

    uint point_geoms_back_index,
         meshes_back_index,
         ui_rects_back_index,
         lights_back_index,
         materials_back_index;
    
    Spatial camera_prev,
            camera_cur;
    float   camera_prev_fov,
            camera_cur_fov,
            camera_near,
            camera_far;

    uint width,
         height;

  public:
    void begin_point_geom (IxGeomCompilation* comp, Spatial prev, Spatial cur)
    {
      PointSpatial spat = { prev, cur };
      point_spats [point_geoms_back_index] = spat;
      
      PointGeom geom = { 0, 0 };
      point_geoms [point_geoms_back_index] = geom;
      
      point_comps [point_geoms_back_index] = comp;
    }
    
    void add_meshes (const Mesh* meshes_in, uint count)
    {
      if (meshes_back_index == max_meshes)
        return;
      
      for (uint i = 0; i != count; i++)
        meshes [meshes_back_index + i] = meshes_in [i];
      meshes_back_index += count;
      
      point_geoms [point_geoms_back_index].mesh_count += count;
    }
    
    void add_materials (const Material* materials_in, uint count)
    {
      if (materials_back_index == max_materials)
        return;
      
      if (materials_in)
      {
        for (uint i = 0; i != count; i++)
          materials [materials_back_index + i] = materials_in [i];
      }
      
      materials_back_index += count;
      
      point_geoms [point_geoms_back_index].material_count += count;
    }
    
    void end_point_geom ()
    {
      point_geoms_back_index++;
    }
    
    /*void add_ui_rects (const UIRect* rects_in, u32 count)
    {
      for (uint i = 0; i != count; i++)
        ui_rects [ui_rects_back_index + i] = rects_in [i];
      ui_rects_back_index += count;
    }
    
    void add_light (Rk::Vector3f position, Rk::Vector3f colour)
    {
      if (lights_back_index != max_lights)
        lights [lights_back_index++] = Light { Rk::Vector4f (position, 1), Rk::Vector4f (colour, 1) };
    }*/
    
    void set_camera (Spatial prev, Spatial cur, float prev_fov, float cur_fov, float near, float far)
    {
      camera_prev     = prev;
      camera_cur      = cur;
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

  }; // class Frame

} // namespace Co

#endif
