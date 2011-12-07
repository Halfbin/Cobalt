//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLFrame.hpp"

// Uses
#include <Rk/Matrix.hpp>
#include <Rk/Lerp.hpp>

#include "GLCompilation.hpp"
#include "GLTexImage.hpp"
#include "GL.hpp"

namespace Co
{
  bool GLFrame::begin_point_geom (IxGeomCompilation* compilation, Spatial prev, Spatial current)
  {
    if (point_geoms_back_index == max_point_geoms)
      return false;

    PointSpatial spat = { prev, current };
    point_spats [point_geoms_back_index] = spat;
      
    PointGeom geom = { 0, 0 };
    point_geoms [point_geoms_back_index] = geom;
      
    point_comps [point_geoms_back_index] = compilation;

    return true;
  }

  void GLFrame::end_point_geom ()
  {
    point_geoms_back_index++;
  }

  void GLFrame::add_meshes (const Mesh* begin, const Mesh* end)
  {
    uint old_meshes_back_index = meshes_back_index;

    while (meshes_back_index != max_meshes && begin != end)
      meshes [meshes_back_index++] = *begin++;
    
    point_geoms [point_geoms_back_index].mesh_count += meshes_back_index - old_meshes_back_index;
  }

  void GLFrame::add_materials (const Material* begin, const Material* end)
  {
    uint old_materials_back_index = materials_back_index;

    while (materials_back_index != max_materials && begin != end)
      materials [materials_back_index++] = *begin++;
    
    point_geoms [point_geoms_back_index].material_count += materials_back_index - old_materials_back_index;
  }

  void GLFrame::add_ui_batch (IxTexImage* texture, const TexRect* begin, const TexRect* end)
  {
    if (ui_batches_back_index == max_ui_batches)
      return;

    auto& batch = ui_batches [ui_batches_back_index];
    batch.tex = texture;
    batch.first = tex_rects_back_index;

    while (tex_rects_back_index != max_tex_rects && begin != end)
      tex_rects [tex_rects_back_index++] = *begin++;

    batch.count = tex_rects_back_index - batch.first;

    ui_batches_back_index++;
  }

  void GLFrame::add_label (IxTexImage* texture, Spatial prev, Spatial current, const TexRect* begin, const TexRect* end)
  {
    if (labels_back_index == max_labels)
      return;

    auto& label = labels [labels_back_index];
    label.tex = texture;
    label.first = tex_rects_back_index;

    while (tex_rects_back_index != max_tex_rects && begin != end)
      tex_rects [tex_rects_back_index++] = *begin++;

    label.count = tex_rects_back_index - label.first;

    label_spats [labels_back_index].prev = prev;
    label_spats [labels_back_index].cur  = current;
      
    labels_back_index++;
  }

  void GLFrame::add_lights (const Light* begin, const Light* end)
  {
    while (lights_back_index != max_lights && begin != end)
      lights [lights_back_index++] = *begin++;
  }

  void GLFrame::set_camera (Spatial prev, Spatial cur, float prev_fov, float cur_fov, float near, float far)
  {
    camera_prev     = prev;
    camera_cur      = cur;
    camera_prev_fov = prev_fov;
    camera_cur_fov  = cur_fov;
    camera_near     = near;
    camera_far      = far;
  }

  void GLFrame::set_size (u32 w, u32 h)
  {
    width  = w;
    height = h;
  }

  void GLFrame::set_skybox (IxTexImage* cube, Co::Vector3 colour, float prev_alpha, float cur_alpha)
  {
    skybox_tex         = static_cast <GLTexImage*> (cube);
    skybox_colour      = colour;
    skybox_prev_alpha  = prev_alpha;
    skybox_cur_alpha   = cur_alpha;
  }

  //
  // render_point_geoms
  //
  void GLFrame::render_point_geoms (Rk::Matrix4f world_to_clip, GeomProgram& geom_program, float alpha)
  {
    glEnable (GL_DEPTH_TEST);
    //glDisable (GL_CULL_FACE);

    Spatial point_interps [max_point_geoms];
    for (uint i = 0; i != point_geoms_back_index; i++)
      point_interps [i] = lerp (point_spats [i].prev, point_spats [i].cur, alpha);

    uint cur_mesh = 0,
         cur_mat  = 0;

    GLCompilation* prev_comp = 0;

    for (uint geom_index = 0; geom_index != point_geoms_back_index; geom_index++)
    {
      GLCompilation* comp = static_cast <GLCompilation*> (point_comps [geom_index]);

      if (comp != prev_comp)
      {
        if (prev_comp)
          prev_comp -> done ();
        comp -> use ();
        prev_comp = comp;
      }

      static const GLenum index_types [4] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
      static const uptr   index_sizes [4] = { 0, 1, 2, 4 };
      
      IndexType comp_index_type = comp -> get_index_type ();
      GLenum index_type = index_types [comp_index_type];
      uptr   index_size = index_sizes [comp_index_type];

      geom_program.set_model_to_clip (
        world_to_clip *
        Rk::affine_xform (
          point_interps [geom_index].position,
          point_interps [geom_index].orientation
        )
      );
      
      const uint end_mesh = cur_mesh + point_geoms [geom_index].mesh_count;
      while (cur_mesh != end_mesh)
      {
        const Mesh& mesh = meshes [cur_mesh++];

        auto tex = static_cast <GLTexImage*> (materials [cur_mat + mesh.material].diffuse_tex);
        if (tex)
          tex -> bind (geom_program.texunit_diffuse);
        else
          GLTexImage::unbind (geom_program.texunit_diffuse);

        static const GLenum prim_types [7] = {
          GL_POINTS,
          GL_LINES,
          GL_LINE_LOOP,
          GL_LINE_STRIP,
          GL_TRIANGLES,
          GL_TRIANGLE_STRIP,
          GL_TRIANGLE_FAN
        };
        GLenum prim_type = prim_types [mesh.prim_type];

        if (index_type)
        {
          uptr offset = index_size * mesh.base_index;
          glDrawElementsBaseVertex (prim_type, mesh.element_count, index_type, (void*) offset, mesh.base_element);
          check_gl ("glDrawElementsBaseVertex");
        }
        else
        {
          glDrawArrays (prim_type, mesh.base_element, mesh.element_count);
          check_gl ("glDrawArrays");
        }
      } // while (meshes)
      
      cur_mat += point_geoms [geom_index].material_count;
    } // for (point_geoms)

    GLCompilation::done ();
  }

  //
  // render_labels
  //
  void GLFrame::render_labels (float alpha)
  {
    // Interpolate label origins
    /*Spatial label_interps [max_labels];
    for (uint i = 0; i != labels_back_index; i++)
      label_interps [i] = lerp (label_spats [i].prev, label_spats [i].cur, alpha);*/

    // Project origins

  }

  //
  // render_ui_batches
  //
  void GLFrame::render_ui_batches (RectProgram& rect_program)
  {
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (uint index = 0; index != ui_batches_back_index; index++)
    {
      auto batch = ui_batches [index];

      if (batch.tex)
        static_cast <GLTexImage*> (batch.tex) -> bind (rect_program.texunit_tex);
      else
        GLTexImage::unbind (rect_program.texunit_tex);

      float mat [4][4] = {
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f }
      };

      Rk::Matrix4f colour_trans (mat);

      rect_program.set_tex_to_colour (colour_trans);

      uptr offset = batch.first * 32;

      //glDrawArraysInstancedBaseInstance ( // DAMNIT

      glVertexAttribIPointer (rect_program.attrib_rect, 4, GL_INT, 32, (void*) uptr (offset + 0));
      check_gl ("glVertexAttribIPointer");

      glVertexAttribIPointer (rect_program.attrib_tcoords, 4, GL_INT, 32, (void*) uptr (offset + 16));
      check_gl ("glVertexAttribIPointer");

      glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, batch.count);
      check_gl ("glDrawArraysInstanced");
    }

    glDisable (GL_BLEND);
  }

  //
  // render
  // Renders a frame. This is where the interesting stuff happens.
  //
  void GLFrame::render (float alpha, SkyboxProgram& skybox_program, GeomProgram& geom_program, RectProgram& rect_program)
  {
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Transformations
    Spatial camera_sp = lerp (camera_prev, camera_cur, alpha);
    
    auto world_to_eye = Rk::world_to_eye_xform (
      camera_sp.position,
      camera_sp.orientation
    );
    
    float aspect = float (width) / float (height);

    auto eye_to_clip = Rk::eye_to_clip_xform (
      Rk::lerp (camera_prev_fov / aspect, camera_cur_fov / aspect, alpha),
      aspect,
      camera_near, camera_far
    );
    
    auto world_to_clip = eye_to_clip * world_to_eye;
    
    auto ui_to_clip = Rk::ui_to_clip_xform (
      float (width),
      float (height)
    );
    
    // Render skybox
    if (skybox_tex)
    {
      skybox_tex -> bind (skybox_program.texunit_cube);
      float skybox_alpha = Rk::lerp (skybox_prev_alpha, skybox_cur_alpha, alpha);
      skybox_program.render (world_to_clip, skybox_colour, skybox_alpha);
    }
    
    // Render point geometries
    geom_program.use ();
    render_point_geoms (world_to_clip, geom_program, alpha);
    geom_program.done ();

    // Render textured rectangles
    TexRect adjusted_rects [max_tex_rects];

    for (uint index = 0; index != tex_rects_back_index; index++)
    {
      auto& rect = tex_rects [index];
      adjusted_rects [index] = TexRect (rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, rect.s, rect.t, rect.s + rect.tw, rect.t + rect.th);
    }

    rect_program.use ();
    rect_program.set_ui_to_clip (ui_to_clip);
    rect_program.upload_rects (adjusted_rects, tex_rects_back_index);

    //render_labels (alpha);
    render_ui_batches (rect_program);

    rect_program.done ();
    
    // Clean up old VAOs
    if (garbage_vao_back_index != 0)
    {
      glDeleteVertexArrays (garbage_vao_back_index, garbage_vaos);
      check_gl ("glDeleteVertexArrays");
    }
  } // GLFrame::render

  u32 GLFrame::reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id)
  {
    prev_time = new_prev_time;
    time      = new_current_time;

    point_geoms_back_index = 0;
    meshes_back_index      = 0;
    labels_back_index      = 0;
    ui_batches_back_index  = 0;
    lights_back_index      = 0;
    materials_back_index   = 0;
    garbage_vao_back_index = 0;
    tex_rects_back_index   = 0;

    u32 old_id = id;
    new_id = (id += id_advance);
    return old_id;
  }

} // namespace Co
