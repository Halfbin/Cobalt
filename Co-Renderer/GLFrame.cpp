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
  void GLFrame::render_point_geoms (GeomProgram& geom_program, float alpha)
  {
    Spatial point_interps [max_point_geoms];
    for (uint i = 0; i != point_geoms_back_index; i++)
      point_interps [i] = lerp (point_spats [i].prev, point_spats [i].cur, alpha);

    uint cur_mesh = 0,
         cur_mat  = 0;

    geom_program.use ();
    /*glEnableVertexAttribArray (attrib_position);
    glEnableVertexAttribArray (attrib_tcoords);
    glEnableVertexAttribArray (attrib_normal);*/

    for (uint geom_index = 0; geom_index != point_geoms_back_index; geom_index++)
    {
      auto comp = static_cast <GLCompilation*> (point_comps [geom_index]);
      comp -> use ();

      geom_program.set_model_to_world (
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
        {
          tex -> bind (texunit_diffuse);
        }
        else
        {
          glActiveTexture (GL_TEXTURE0 + texunit_diffuse);
          check_gl ("glActiveTexture");

          glBindTexture (GL_TEXTURE_2D, 0);
          check_gl ("glBindTexture");
        }

        static const GLenum gl_prim_types [7] = {
          GL_POINTS,
          GL_LINES,
          GL_LINE_LOOP,
          GL_LINE_STRIP,
          GL_TRIANGLES,
          GL_TRIANGLE_STRIP,
          GL_TRIANGLE_FAN
        };
        GLenum prim_type = gl_prim_types [mesh.prim_type];

        if (mesh.index_type)
        {
          static const GLenum gl_index_types [4] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
          GLenum index_type = gl_index_types [mesh.index_type];
          
          uptr offset = uptr (mesh.first_item) << uint (mesh.index_type - 1);
          
          glDrawElements (prim_type, mesh.element_count, index_type, (void*) offset);
          check_gl ("glDrawElements");
        }
        else
        {
          glDrawArrays (prim_type, mesh.first_item, mesh.element_count);
          check_gl ("glDrawArrays");
        }
      } // while (meshes)
      
      cur_mat += point_geoms [geom_index].material_count;
    } // for (point_geoms)

    geom_program.done ();
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
  void GLFrame::render_ui_batches ()
  {

  }

  //
  // render
  // Renders a frame. This is where the interesting stuff happens.
  //
  void GLFrame::render (float alpha, GeomProgram& geom_program, RectProgram& rect_program)
  {
    glClearColor (0.0f, 0.03f, 0.2f, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Transformations
    Spatial camera_sp = lerp (camera_pos.prev, camera_pos.cur, alpha);
    
    auto world_to_eye = Rk::world_to_eye_xform (
      camera_sp.position,
      camera_sp.orientation
    );
    
    auto eye_to_clip = Rk::eye_to_clip_xform (
      Rk::lerp (camera_prev_fov, camera_cur_fov, alpha),
      float (width) / float (height),
      camera_near, camera_far
    );
    
    auto world_to_clip = eye_to_clip * world_to_eye;
    
    auto ui_to_clip = Rk::ui_to_clip_xform (
      float (width),
      float (height)
    );
    
    geom_program.set_world_to_clip (world_to_clip);
    geom_program.set_world_to_eye  (world_to_eye);

    // Render point geometries
    render_point_geoms (geom_program, alpha);

    // Clean up old VAOs
    glDeleteVertexArrays (garbage_vao_back_index, garbage_vaos);
    check_gl ("glDeleteVertexArrays");

    // Render textured rectangles
    for (uint index = 0; index != tex_rects_back_index; index++)
    {
      auto& rect = tex_rects [index];
      rect.w  += rect.x;
      rect.h  += rect.y;
      rect.tw += rect.s;
      rect.th += rect.t;
    }

    render_labels (alpha);
    render_ui_batches ();
  } // GLFrame::render

  u32 GLFrame::reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id)
  {
    prev_time = new_prev_time;
    time      = new_current_time;

    point_geoms_back_index = 0;
    meshes_back_index      = 0;
    ui_batches_back_index  = 0;
    lights_back_index      = 0;
    materials_back_index   = 0;
    garbage_vao_back_index = 0;

    u32 old_id = id;
    new_id = (id += id_advance);
    return old_id;
  }

} // namespace Co
