//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLFRAME
#define CO_GLRENDERER_H_GLFRAME

// Implements
#include <Co/IxFrame.hpp>

// Uses
#include <Rk/MatrixForward.hpp>

#include "SkyboxProgram.hpp"
#include "GeomProgram.hpp"
#include "RectProgram.hpp"
#include "GLTexImage.hpp"

namespace Co
{
  //
  // = GLFrame =========================================================================================================
  //
  class GLFrame :
    public IxFrame
  {
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
    enum
    {
      max_point_geoms = 5000,
      max_meshes      = 5000,
      max_materials   = 5000,
      max_ui_batches  = 2000,
      max_labels      = 2000,
      max_tex_rects   = 10000,
      max_lights      = 8
    };
    
    // Data
    PointSpatial       point_spats [max_point_geoms];
    PointGeom          point_geoms [max_point_geoms];
    IxGeomCompilation* point_comps [max_point_geoms];
    Mesh               meshes      [max_meshes];
    Material           materials   [max_materials];
    UIBatch            ui_batches  [max_ui_batches];
    Label              labels      [max_labels];
    PointSpatial       label_spats [max_labels];
    TexRect            tex_rects   [max_tex_rects];
    Light              lights      [max_lights];

    // Skybox
    GLTexImage* skybox_tex;
    Vector3     skybox_colour;
    float       skybox_prev_alpha,
                skybox_cur_alpha;

    // Indices
    u32 point_geoms_back_index,
        meshes_back_index,
        materials_back_index,
        ui_batches_back_index,
        labels_back_index,
        tex_rects_back_index,
        lights_back_index;

    // Camera
    Spatial camera_prev,
            camera_cur;
    float   camera_prev_fov,
            camera_cur_fov,
            camera_near,
            camera_far;

    // Viewport
    u32 width,
        height;

    // Rendering
    void render_point_geoms (Rk::Matrix4f model_to_clip, GeomProgram& geom_program, float alpha);
    void render_labels      (float alpha);
    void render_ui_batches  (RectProgram& rect_program);

  public:
    // Timing
    u32 id;
    float time, prev_time;

    // VAO cleanup
    enum Maxima { max_garbage_vaos = 256 };
    u32 garbage_vaos [max_garbage_vaos];
    uint garbage_vao_back_index;

    // Drawing interface
    virtual bool begin_point_geom (IxGeomCompilation* compilation, Spatial prev, Spatial current);
    virtual void end_point_geom   ();
    virtual void add_meshes       (const Mesh* meshes, const Mesh* end);
    virtual void add_materials    (const Material* materials, const Material* end);
    virtual void add_ui_batch     (IxTexImage* texture, const TexRect* rects, const TexRect* end);
    virtual void add_label        (IxTexImage* texture, Spatial prev, Spatial current, const TexRect* begin, const TexRect* end);
    virtual void add_lights       (const Light* lights, const Light* end);
    virtual void set_camera       (Spatial prev, Spatial current, float prev_fov, float current_fov, float near, float far);
    virtual void set_size         (u32 width, u32 height);
    virtual void set_skybox       (IxTexImage* cube, Co::Vector3 colour, float prev_alpha, float alpha);

    // Rendering
    void render (float alpha, SkyboxProgram& skybox_program, GeomProgram& geom_program, RectProgram& rect_program);
    
    // Acquisition
    u32 reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id);

  }; // class GLFrame

} // namespace Co

#endif
