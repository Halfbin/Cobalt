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
          check_gl ("glDrawElements");
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
  void GLFrame::render (float alpha, GeomProgram& geom_program, RectProgram& rect_program)
  {
    glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Transformations
    Spatial camera_sp = lerp (camera_pos.prev, camera_pos.cur, alpha);
    
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
    
    geom_program.use ();
    
    // Render point geometries
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
