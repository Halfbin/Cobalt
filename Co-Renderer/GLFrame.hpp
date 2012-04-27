//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLFRAME
#define CO_GLRENDERER_H_GLFRAME

// Implements
#include <Co/Frame.hpp>

// Uses
#include <Rk/MatrixForward.hpp>

#include "GLCompilation.hpp"
#include "SkyboxProgram.hpp"
#include "GeomProgram.hpp"
#include "RectProgram.hpp"
#include "GLTexImage.hpp"

#include <vector>

namespace Co
{
  //
  // = GLFrame =========================================================================================================
  //
  class GLFrame :
    public Frame
  {
    struct PointGeom
    {
      GLCompilation::Ptr comp;
      Spatial            prev,
                         cur;
      u32                mesh_count,
                         material_count;

      PointGeom (GLCompilation::Ptr comp, Spatial prev, Spatial cur) :
        comp           (std::move (comp)),
        prev           (prev),
        cur            (cur),
        mesh_count     (0),
        material_count (0)
      { }
      
    };

    struct Label
    {
      GLTexImage::Ptr tex;
      u32             first,
                      count;
      Vector4         prev_linear_colour,
                      cur_linear_colour,
                      prev_const_colour,
                      cur_const_colour;

      Label (
        GLTexImage::Ptr tex,
        u32             first,
        u32             count,
        Vector4         prev_linear_colour,
        Vector4         cur_linear_colour,
        Vector4         prev_const_colour,
        Vector4         cur_const_colour
      ) :
        tex                (std::move (tex)),
        first              (first),
        count              (count),
        prev_linear_colour (prev_linear_colour),
        cur_linear_colour  (cur_linear_colour),
        prev_const_colour  (prev_const_colour),
        cur_const_colour   (cur_const_colour)
      { }
      
    };

    struct Label2D :
      Label
    {
      Spatial2D prev,
                current;

      Label2D (
        GLTexImage::Ptr tex,
        u32             first,
        u32             count,
        Spatial2D       prev,
        Spatial2D       current,
        Vector4         prev_linear_colour,
        Vector4         cur_linear_colour,
        Vector4         prev_const_colour,
        Vector4         cur_const_colour
      ) :
        Label   (std::move (tex), first, count, prev_linear_colour, cur_linear_colour, prev_const_colour, cur_const_colour),
        prev    (prev),
        current (current)
      { }
      
    };

    struct Label3D :
      Label
    {
      Spatial prev,
              current;
      
      Label3D (
        GLTexImage::Ptr tex,
        u32             first,
        u32             count,
        Spatial         prev,
        Spatial         current,
        Vector4         prev_linear_colour,
        Vector4         cur_linear_colour,
        Vector4         prev_const_colour,
        Vector4         cur_const_colour
      ) :
        Label   (std::move (tex), first, count, prev_linear_colour, cur_linear_colour, prev_const_colour, cur_const_colour),
        prev    (prev),
        current (current)
      { }
      
    };

    // Data
    std::vector <PointGeom> geoms;
    std::vector <Mesh>      meshes;
    std::vector <Material>  materials;
    std::vector <Label2D>   labels_2d;
    std::vector <Label3D>   labels_3d;
    std::vector <TexRect>   rects;
    std::vector <Light>     lights;

    // Skybox
    GLTexImage::Ptr skybox_tex;
    Vector3         skybox_colour;
    float           skybox_prev_alpha,
                    skybox_cur_alpha;

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
    void render_geoms     (Rk::Matrix4f model_to_clip, GeomProgram& geom_program, float alpha);
    void render_labels_3d (float alpha);
    void render_labels_2d (RectProgram& rect_program, float alpha);

  public:
    GLFrame ();

    // Timing
    float time,
          prev_time;

    virtual uint get_width  ();
    virtual uint get_height ();

    // Drawing interface
    virtual void begin_point_geom (GeomCompilation::Ptr compilation, Spatial prev, Spatial current);
    virtual void end              ();

    virtual void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      const TexRect* end,
      Spatial2D      prev,
      Spatial2D      cur,
      Vector4        prev_linear_colour,
      Vector4        cur_linear_colour,
      Vector4        prev_const_colour,
      Vector4        cur_const_colour
    );

    virtual void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      const TexRect* end,
      Spatial        prev,
      Spatial        cur,
      Vector4        prev_linear_colour,
      Vector4        cur_linear_colour,
      Vector4        prev_const_colour,
      Vector4        cur_const_colour
    );

    virtual void add_lights (const Light* lights, const Light* end);
    virtual void set_skybox (TexImage::Ptr cube, Co::Vector3 colour, float prev_alpha, float alpha);

    virtual void set_camera (Spatial prev, Spatial current, float prev_fov, float current_fov, float near, float far);
    virtual void set_size   (u32 width, u32 height);
    
    virtual void add_meshes    (const Mesh*     begin, const Mesh*     end);
    virtual void add_materials (const Material* begin, const Material* end);
    
    virtual void submit ();

    // Rendering
    void render (float alpha, SkyboxProgram& skybox_program, GeomProgram& geom_program, RectProgram& rect_program);
    
    // Acquisition
    void reset (float new_prev_time, float new_current_time);

  }; // class GLFrame

} // namespace Co

#endif
