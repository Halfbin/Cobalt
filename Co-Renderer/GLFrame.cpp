//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLFrame.hpp"

// Uses
#include <Rk/Exception.hpp>
#include <Rk/Matrix.hpp>
#include <Rk/Lerp.hpp>

#include "GLCompilation.hpp"
#include "GLRenderer.hpp"
#include "GLTexImage.hpp"
#include "GL.hpp"

namespace Co
{
  GLFrame::GLFrame ()
  {
    geoms.reserve     (5000);
    meshes.reserve    (5000);
    materials.reserve (5000);
    labels_2d.reserve (2000);
    labels_3d.reserve (2000);
    rects.reserve     (5000);
    lights.reserve    (8);
  }

  void GLFrame::render_geoms (Rk::Matrix4f world_to_clip, GeomProgram& geom_program, float alpha)
  {
    glEnable (GL_DEPTH_TEST);
    //glDisable (GL_CULL_FACE);

    for (auto geom = geoms.begin (); geom != geoms.end (); geom++)
      geom -> cur = lerp (geom -> prev, geom -> cur, alpha);

    auto cur_mesh = meshes.begin    ();
    auto cur_mat  = materials.begin ();

    //GLCompilation::Ptr prev_comp = nullptr;

    for (auto geom = geoms.begin (); geom != geoms.end (); geom++)
    {
      /*if (geom -> comp != prev_comp)
      {
        if (prev_comp)
          prev_comp -> done ();*/
        bool ok = geom -> comp -> use ();
        /*prev_comp = geom -> comp;
      }*/

      if (!ok)
        renderer.log () << "- uh-oh\n";

      static const GLenum index_types [4] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
      static const uptr   index_sizes [4] = { 0, 1, 2, 4 };
      
      IndexType comp_index_type = geom -> comp -> get_index_type ();
      GLenum index_type = index_types [comp_index_type];
      uptr   index_size = index_sizes [comp_index_type];

      geom_program.set_model_to_clip (
        world_to_clip *
        Rk::affine_xform (
          geom -> cur.position,
          geom -> cur.orientation
        )
      );
      
      auto end_mesh = cur_mesh + geom -> mesh_count;
      while (cur_mesh != end_mesh)
      {
        const Mesh& mesh = *cur_mesh++;

        auto tex = static_cast <GLTexImage*> ((cur_mat + mesh.material) -> diffuse_tex.get ());
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
      
      cur_mat += geom -> material_count;
    } // for (point_geoms)

    GLCompilation::done ();
  }

  void GLFrame::render_labels_3d (float alpha)
  {

  }

  void GLFrame::render_labels_2d (RectProgram& rect_program, float alpha)
  {
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (auto label = labels_2d.begin (); label != labels_2d.end (); label++)
    {
      if (label -> tex)
        label -> tex -> bind (rect_program.texunit_tex);
      else
        GLTexImage::unbind (rect_program.texunit_tex);

      rect_program.set_linear_colour (lerp (label -> prev_linear_colour, label -> cur_linear_colour, alpha));
      rect_program.set_const_colour  (lerp (label -> prev_const_colour,  label -> cur_const_colour,  alpha));
      rect_program.set_transform     (lerp (label -> prev,               label -> current,           alpha));
      uptr offset = label -> first * 32;

      //glDrawArraysInstancedBaseInstance ( // DAMNIT

      glVertexAttribIPointer (rect_program.attrib_rect, 4, GL_INT, 32, (void*) uptr (offset + 0));
      check_gl ("glVertexAttribIPointer");

      glVertexAttribIPointer (rect_program.attrib_tcoords, 4, GL_INT, 32, (void*) uptr (offset + 16));
      check_gl ("glVertexAttribIPointer");

      glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, label -> count);
      check_gl ("glDrawArraysInstanced");
    }

    glDisable (GL_BLEND);
  }

  //
  // = render ==========================================================================================================
  //
  void GLFrame::render (float alpha, SkyboxProgram& skybox_program, GeomProgram& geom_program, RectProgram& rect_program)
  {
    auto back = pow (Vector3 (0.00f, 0.26f, 0.51f), 2.2f);
    glClearColor (back.x, back.y, back.z, 1.0f);
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
      auto sky_to_eye = Rk::world_to_eye_xform (
        v3f (0, 0, 0),
        camera_sp.orientation
      );

      auto sky_to_clip = eye_to_clip * sky_to_eye;

      skybox_tex -> bind (skybox_program.texunit_cube);
      float skybox_alpha = Rk::lerp (skybox_prev_alpha, skybox_cur_alpha, alpha);
      skybox_program.render (sky_to_clip, skybox_colour, skybox_alpha);
    }
    
    // Render point geometries
    geom_program.use ();
    render_geoms (world_to_clip, geom_program, alpha);
    geom_program.done ();

    // Adjust textured rectangles
    for (auto rect = rects.begin (); rect != rects.end (); rect++)
    {
      *rect = TexRect (
        rect -> x,
        rect -> y,
        rect -> x + rect -> w,
        rect -> y + rect -> h,
        rect -> s,
        rect -> t,
        rect -> s + rect -> tw,
        rect -> t + rect -> th
      );
    }

    // Render rectangles
    rect_program.use ();
    rect_program.set_ui_to_clip (ui_to_clip);
    rect_program.upload_rects (rects.data (), rects.size ());

    //render_labels_3d (alpha);
    render_labels_2d (rect_program, alpha);

    rect_program.done ();
  }

  uint GLFrame::get_width ()
  {
    return width;
  }

  uint GLFrame::get_height ()
  {
    return height;
  }

  //
  // = Geometry stuff ==================================================================================================
  //
  void GLFrame::begin_point_geom (GeomCompilation::Ptr comp, Spatial prev, Spatial current)
  {
    geoms.push_back (
      PointGeom (
        std::static_pointer_cast <GLCompilation> (comp),
        prev,
        current
      )
    );
  }

  void GLFrame::add_meshes (const Mesh* begin, const Mesh* end)
  {
    if (geoms.empty ())
      throw std::logic_error ("No current geom");
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;
    
    meshes.insert (meshes.end (), begin, end);
    geoms.back ().mesh_count += size;
  }

  void GLFrame::add_materials (const Material* begin, const Material* end)
  {
    if (geoms.empty ())
      throw std::logic_error ("No current geom");
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;

    materials.insert (materials.end (), begin, end);
    geoms.back ().material_count += size;
  }

  void GLFrame::end ()
  {

  }

  void GLFrame::add_label (
    TexImage::Ptr  texture,
    const TexRect* begin,
    const TexRect* end,
    Spatial2D      prev,
    Spatial2D      cur,
    Vector4        prev_linear_colour,
    Vector4        cur_linear_colour,
    Vector4        prev_const_colour,
    Vector4        cur_const_colour)
  {
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;

    labels_2d.push_back (
      Label2D (
        std::static_pointer_cast <GLTexImage> (texture),
        rects.size (),
        size,
        prev,
        cur,
        prev_linear_colour,
        cur_linear_colour,
        prev_const_colour,
        cur_const_colour
      )
    );
    rects.insert (rects.end (), begin, end);
  }
  
  void GLFrame::add_label (
    TexImage::Ptr  texture,
    const TexRect* begin,
    const TexRect* end,
    Spatial        prev,
    Spatial        cur,
    Vector4        prev_linear_colour,
    Vector4        cur_linear_colour,
    Vector4        prev_const_colour,
    Vector4        cur_const_colour)
  {
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;

    labels_3d.push_back (
      Label3D (
        std::static_pointer_cast <GLTexImage> (texture),
        rects.size (),
        size,
        prev,
        cur,
        prev_linear_colour,
        cur_linear_colour,
        prev_const_colour,
        cur_const_colour
      )
    );
    rects.insert (rects.end (), begin, end);
  }

  void GLFrame::add_lights (const Light* begin, const Light* end)
  {
    if (!Rk::check_range (begin, end))
      return;

    lights.insert (lights.end (), begin, end);
  }
  
  void GLFrame::set_skybox (TexImage::Ptr cube, Co::Vector3 colour, float prev_alpha, float cur_alpha)
  {
    skybox_tex        = std::static_pointer_cast <GLTexImage> (cube);
    skybox_colour     = colour;
    skybox_prev_alpha = prev_alpha;
    skybox_cur_alpha  = cur_alpha;
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
  
  void GLFrame::set_size (u32 new_width, u32 new_height)
  {
    width  = new_width;
    height = new_height;
  }
  
  void GLFrame::submit ()
  {
    renderer.submit_frame (this);
  }
  
  void GLFrame::reset (float new_prev_time, float new_cur_time)
  {
    geoms.clear ();
    meshes.clear ();
    materials.clear ();
    labels_2d.clear ();
    labels_3d.clear ();
    rects.clear ();
    lights.clear ();

    skybox_tex = nullptr;
    
    prev_time = new_prev_time;
    time      = new_cur_time;
  }

} // namespace Co
