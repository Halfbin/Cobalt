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
  // Render
  // Renders a frame. This is where the interesting stuff happens.
  //
  void GLFrame::render (float alpha, u32 model_to_world_loc, u32 world_to_clip_loc, u32 world_to_eye_loc)
  {
#if opengl_compat
    glClearColor (0.0f, 0.03f, 0.2f, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Spatial camera_sp = lerp (camera_prev, camera_cur, alpha);

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

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    glMatrixMode (GL_MODELVIEW);
    glLoadTransposeMatrixf (world_to_clip.raw ());

    glColor3f (1.0f, 0.0f, 0.0f);
    glBegin (GL_QUADS);
      glVertex3f (10.0f,  4.0f, -4.0f);
      glVertex3f (10.0f, -4.0f, -4.0f);
      glVertex3f (10.0f, -4.0f,  4.0f);
      glVertex3f (10.0f,  4.0f,  4.0f);
    glEnd ();

    /*glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    auto eye_to_clip = Rk::eye_to_clip_xform (
      Rk::lerp (camera_prev_fov, camera_cur_fov, alpha),
      float (width) / float (height),
      camera_near, camera_far
    );
    gluPerspective (
      Rk::lerp (camera_prev_fov, camera_cur_fov, alpha),
      float (width) / float (height),
      camera_near, camera_far
    );
    glLoadTransposeMatrixf (eye_to_clip.raw ());

    Spatial camera_sp = lerp (camera_prev, camera_cur, alpha);
    
    auto world_to_eye = Rk::world_to_eye_xform (
      camera_sp.position,
      camera_sp.orientation
    );

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glLoadTransposeMatrixf (world_to_eye.raw ());
    */

    /*Spatial point_interps [max_point_geoms];
    for (uint i = 0; i != point_geoms_back_index; i++)
      point_interps [i] = lerp (point_spats [i].prev, point_spats [i].cur, alpha);

    uint cur_mesh = 0,
          cur_mat  = 0;

    glEnableClientState (GL_VERTEX_ARRAY);

    for (uint geom_index = 0; geom_index != point_geoms_back_index; geom_index++)
    {
      auto comp = static_cast <GLCompilation*> (point_comps [geom_index]);
      comp -> use ();

      auto model_to_world = Rk::affine_xform (
        point_interps [geom_index].position,
        point_interps [geom_index].orientation
      );

      glPushMatrix ();
      glMultTransposeMatrixf (model_to_world.raw ());

      const uint end_mesh = cur_mesh + point_geoms [geom_index].mesh_count;
      while (cur_mesh != end_mesh)
      {
        const Mesh& mesh = meshes [cur_mesh++];

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
      
      glPopMatrix ();

    } // for (point_geoms)

    glDisableClientState (GL_VERTEX_ARRAY);*/
#else // !opengl_compat
    glClearColor (0.0f, 0.03f, 0.2f, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Transformations
    Spatial camera_sp = lerp (camera_prev, camera_cur, alpha);
    
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
    
    glUniformMatrix4fv (world_to_clip_loc, 1, true, world_to_clip.raw ());
    glUniformMatrix4fv (world_to_eye_loc,  1, true, world_to_eye.raw  ());

    //
    // Render point geoms
    //
    Spatial point_interps [max_point_geoms];
    for (uint i = 0; i != point_geoms_back_index; i++)
      point_interps [i] = lerp (point_spats [i].prev, point_spats [i].cur, alpha);

    uint cur_mesh = 0,
         cur_mat  = 0;

    glEnableVertexAttribArray (attrib_position);
    glEnableVertexAttribArray (attrib_tcoords);
    glEnableVertexAttribArray (attrib_normal);

    for (uint geom_index = 0; geom_index != point_geoms_back_index; geom_index++)
    {
      auto comp = static_cast <GLCompilation*> (point_comps [geom_index]);
      comp -> use ();

      auto model_to_world = Rk::affine_xform (
        point_interps [geom_index].position,
        point_interps [geom_index].orientation
      );
      
      glUniformMatrix4fv (model_to_world_loc, 1, true, model_to_world.raw ());

      const uint end_mesh = cur_mesh + point_geoms [geom_index].mesh_count;
      while (cur_mesh != end_mesh)
      {
        const Mesh& mesh = meshes [cur_mesh++];

        auto tex = static_cast <GLTexImage*> (materials [cur_mat + mesh.material].diffuse_tex);
        tex -> bind (texunit_diffuse);

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

    //
    // Clean up old VAOs
    //
    glDeleteVertexArrays (garbage_vao_back_index, garbage_vaos);
#endif
  } // GLFrame::render

  u32 GLFrame::reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id)
  {
    prev_time = new_prev_time;
    time      = new_current_time;

    point_geoms_back_index = 0;
    meshes_back_index      = 0;
    ui_rects_back_index    = 0;
    lights_back_index      = 0;
    materials_back_index   = 0;
    garbage_vao_back_index = 0;

    u32 old_id = id;
    new_id = (id += id_advance);
    return old_id;
  }

} // namespace Co
