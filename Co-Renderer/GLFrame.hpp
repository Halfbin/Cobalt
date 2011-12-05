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

#include "GeomProgram.hpp"
#include "RectProgram.hpp"

namespace Co
{
  //
  // = GLFrame =========================================================================================================
  //
  class GLFrame :
    public IxFrame
  {
    enum
    {
      max_point_geoms_def = 5000,
      max_meshes_def      = 5000,
      max_materials_def   = 5000,
      max_ui_batches_def  = 2000,
      max_labels_def      = 2000,
      max_tex_rects_def   = 10000,
      max_lights_def      = 8
    };

    PointSpatial       point_spats_buffer [max_point_geoms_def];
    PointGeom          point_geoms_buffer [max_point_geoms_def];
    IxGeomCompilation* point_comps_buffer [max_point_geoms_def];
    Mesh               meshes_buffer      [max_meshes_def];
    Material           materials_buffer   [max_materials_def];
    UIBatch            ui_batches_buffer  [max_ui_batches_def];
    Label              labels_buffer      [max_labels_def];
    PointSpatial       label_spats_buffer [max_labels_def];
    TexRect            tex_rects_buffer   [max_tex_rects_def];
    Light              lights_buffer      [max_lights_def];

    void render_point_geoms (Rk::Matrix4f model_to_clip, GeomProgram& geom_program, float alpha);
    void render_labels      (float alpha);
    void render_ui_batches  (RectProgram& rect_program);

  public:
    GLFrame ();

    uint id;
    float time, prev_time;

    enum Maxima { max_garbage_vaos = 256 };
    u32 garbage_vaos [max_garbage_vaos];
    uint garbage_vao_back_index;

    void render (float alpha, GeomProgram& geom_program, RectProgram& rect_program);
    
    u32 reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id);

  }; // class GLFrame

} // namespace Co

#endif
