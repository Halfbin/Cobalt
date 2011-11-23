//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_FRAME
#define CO_H_FRAME

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
    u32 x,  y,
        w,  h,
        s,  t,
        tw, th;
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
  // = Frame ===========================================================================================================
  //
  class Frame :
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
      u32         x,     //  8 12
                  y;     // 12 16
      u32         first, // 16 20
                  count; // 20 24
    };

    struct Label
    {
      IxTexImage* tex;   //  4  8
      u32         first, //  8 12
                  count; // 12 16
    };

    // Point geometry data
    enum { max_point_geoms = 256 };
    PointSpatial       point_spats [max_point_geoms];
    PointGeom          point_geoms [max_point_geoms];
    IxGeomCompilation* point_comps [max_point_geoms];
    uint               point_geoms_back_index;

    // Meshes
    enum { max_meshes = 1024 };
    Mesh meshes [max_meshes];
    uint meshes_back_index;

    // Materials
    enum { max_materials = 1024 };
    Material materials [max_materials];
    uint     materials_back_index;

    // UI rects
    enum { max_ui_batches = 1024 };
		UIBatch ui_batches [max_ui_batches];
    uint    ui_rects_back_index;

    // Labels
    enum { max_labels = 1024 };
    Label        labels      [max_labels];
    PointSpatial label_spats [max_labels];
    uint         labels_back_index;
    
    // Tex rects
    enum { max_tex_rects = 2048 };
    TexRect tex_rects [max_tex_rects];
    uint    tex_rects_back_index;
    
    // Lights
    enum { max_lights = 8 };
    Light lights [max_lights];
    uint  lights_back_index;
    
    // Camera
    PointSpatial camera_pos;
    float        camera_prev_fov,
                 camera_cur_fov,
                 camera_near,
                 camera_far;

    // Viewport
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
      
      point_geoms [point_geoms_back_index].material_count += old_materials_back_index - materials_back_index;
    }
    
    template <typename Iter>
    void add_materials (Iter in, uint count)
    {
      add_materials (in, in + count);
    }

    template <typename Iter>
    void add_ui_batch (IxTexImage* tex, uint x, uint y, Iter begin, Iter end)
    {
      if (ui_batches_back_index = max_ui_batches)
        return;

      auto& batch = ui_batches [ui_batches_back_index];
      batch.tex = tex;
      batch.x = x;
      batch.y = y;
      batch.first = tex_rects_back_index;

      while (tex_rects_back_index != max_tex_rects && begin != end)
        tex_rects [tex_rects_back_index++] = *begin++;

      batch.count = tex_rects_back_index - batch.first;

      ui_batches_back_index++;
    }

    template <typename Iter>
    void add_ui_batch (IxTexImage* tex, uint x, uint y, Iter begin, uint count)
    {
      add_ui_batch (tex, x, y, begin, begin + count);
    }

    template <typename Iter>
    void add_label (IxTexImage* tex, Spatial prev, Spatial cur, Iter begin, Iter end)
    {
      if (labels_back_index = max_labels)
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

  }; // class Frame

} // namespace Co

#endif
