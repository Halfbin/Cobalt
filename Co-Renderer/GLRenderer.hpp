//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLRENDERER
#define CO_GLRENDERER_H_GLRENDERER

// Implements
#include <Co/Renderer.hpp>

// Uses
#include <Rk/FixedQueue.hpp>
#include <Rk/Thread.hpp>
#include <Rk/Mutex.hpp>

#include "SkyboxProgram.hpp"
#include "GeomProgram.hpp"
#include "RectProgram.hpp"

#include "GLCompilation.hpp"
#include "GLTexImage.hpp"
#include "GLContext.hpp"

#include <vector>

namespace Co
{
  //
  // = GLRenderer ======================================================================================================
  //
  class GLRenderer :
    public Renderer
  {
    // Frame data
    struct PointGeom
    {
      GLCompilation::Ptr comp;
      Spatial            spat;
      u32                mesh_count,
                         material_count;

      PointGeom (GLCompilation::Ptr comp, Spatial spat) :
        comp           (std::move (comp)),
        spat           (spat),
        mesh_count     (0),
        material_count (0)
      { }
      
    };

    struct Label
    {
      GLTexImage::Ptr tex;
      u32             first,
                      count;
      v4f             linear_colour,
                      const_colour;

      Label (
        GLTexImage::Ptr tex,
        u32             first,
        u32             count,
        v4f             linear_colour,
        v4f             const_colour
      ) :
        tex            (std::move (tex)),
        first          (first),
        count          (count),
        linear_colour  (linear_colour),
        const_colour   (const_colour)
      { }
      
    };

    struct Label2D :
      Label
    {
      Spatial2D spat;
      
      Label2D (
        GLTexImage::Ptr tex,
        u32             first,
        u32             count,
        Spatial2D       spat,
        v4f             linear_colour,
        v4f             const_colour
      ) :
        Label (std::move (tex), first, count, linear_colour, const_colour),
        spat  (spat)
      { }
      
    };

    struct Label3D :
      Label
    {
      Spatial spat;
      
      Label3D (
        GLTexImage::Ptr tex,
        u32             first,
        u32             count,
        Spatial         spat,
        v4f             linear_colour,
        v4f             const_colour
      ) :
        Label (std::move (tex), first, count, linear_colour, const_colour),
        spat  (spat)
      { }
      
    };

    std::vector <PointGeom>  geoms;
    std::vector <Mesh>       meshes;
    std::vector <Material>   materials;
    std::vector <Label2D>    labels_2d;
    std::vector <Label3D>    labels_3d;
    std::vector <TexRect>    rects;
    std::vector <Light>      lights;

    // Skybox
    GLTexImage::Ptr skybox_tex;
    v3f             skybox_colour;
    float           skybox_alpha;

    // Camera
    Spatial camera_spat;
    float   camera_fov,
            camera_near,
            camera_far;

    // Render device
    void*          target;
    void*          shared_dc;
    int            format;
    void*          shared_rc;
    WGLCCA         wglCreateContextAttribs;
    Rk::Mutex      device_mutex;
    GLContext::Ptr true_context;

    // Subsystems
    const Clock*       clock;
    Log*               log_ptr;
    GeomProgram::Ptr   geom_program;
    RectProgram::Ptr   rect_program;
    SkyboxProgram::Ptr skybox_program;

    // Setup and teardown
    virtual void init    (void* hwnd, const Clock& clock, Log& log);
    void         cleanup ();
    
    // Frame exchange
    virtual void begin_point_geom (GeomCompilation::Ptr compilation, Spatial spat);
    virtual void end              ();

    virtual void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      const TexRect* end,
      Spatial2D      spat,
      v4f            linear_colour,
      v4f            const_colour
    );

    virtual void add_label (
      TexImage::Ptr  texture,
      const TexRect* rects,
      const TexRect* end,
      Spatial        spat,
      v4f            linear_colour,
      v4f            const_colour
    );

    virtual void add_lights (const Light* lights, const Light* end);
    virtual void set_skybox (TexImage::Ptr cube, v3f colour, float alpha);

    virtual void set_camera (Spatial spat, float fov, float near, float far);
    
    virtual void add_meshes    (const Mesh*     begin, const Mesh*     end);
    virtual void add_materials (const Material* begin, const Material* end);
    
    virtual void render_frame ();

    // Rendering
    void render_geoms     (Rk::Matrix4f model_to_clip);
    void render_labels_3d ();
    void render_labels_2d ();

    // Device
    GLContext::Ptr             create_context_impl ();
    virtual RenderContext::Ptr create_context      ();
    
    GLRenderer ();

  public:
    Log::Lock log ()
    {
      return log_ptr -> lock ();
    }

    static GLRenderer instance;

    static std::shared_ptr <GLRenderer> create ();

  }; // class GLRenderer

  static GLRenderer& renderer = GLRenderer::instance;

} // namespace Co

#endif
