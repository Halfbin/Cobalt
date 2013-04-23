//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLRenderer.hpp"

// Uses
#include <Co/GeomCompilation.hpp>
#include <Co/Clock.hpp>

#include <Rk/Modular.hpp>
#include <Rk/Guard.hpp>
#include <Rk/File.hpp>

#include "GLTexImage.hpp"
#include "GL.hpp"

//
// = WGL stuff =========================================================================================================
//
typedef iptr (__stdcall *WndProc) (void*, u32, uptr, iptr);

struct WndClassExW
{
  u32           size,
                style;
  WndProc       window_proc;
  u32           class_extra,
                window_extra;
  void          *instance,
                *icon,
                *cursor,
                *background_brush;
  const wchar_t *menu_name,
                *class_name;
  void*         small_icon;
};

struct PixelFormatDescriptor
{
  i16 size, version;
  u32 flags;
  u8  pixel_type,
      color_bits, red_bits, red_shift, green_bits, green_shift, blue_bits, blue_shift,
      alpha_bits, alpha_shift,
      accum_bits, accum_red_bits, accum_green_bits, accum_blue_bits, accum_alpha_bits,
      depth_bits, stencil_bits,
      aux_buffers,
      layer_type,
      reserved;
  u32 layer_mask, visible_mask, damage_mask;
};

enum PFDConstants
{
  pfd_support_opengl = 0x00000020,
  pfd_draw_to_window = 0x00000004,
  pfd_type_rgba      = 0,
  pfd_main_plane     = 0
};

enum WGLPixelFormatConstants
{
  WGL_DRAW_TO_WINDOW = 0x2001,
  WGL_ACCELERATION   = 0x2003,
  //WGL_SWAP_METHOD    = 0x2007,
  WGL_SUPPORT_OPENGL = 0x2010,
  WGL_DOUBLE_BUFFER  = 0x2011,
  WGL_PIXEL_TYPE     = 0x2013,
  WGL_COLOR_BITS     = 0x2014,
  WGL_ALPHA_BITS     = 0x201b,
  WGL_DEPTH_BITS     = 0x2022,
  WGL_SAMPLE_BUFFERS = 0x2041,
  WGL_SAMPLES        = 0x2042,

  WGL_FULL_ACCELERATION = 0x2027,
  //WGL_SWAP_EXCHANGE     = 0x2028,
  WGL_TYPE_RGBA         = 0x202b
};

typedef void (__stdcall *GLFunction) ();

extern "C"
{
  #define fromdll __declspec(dllimport)
  
  fromdll GLFunction __stdcall wglGetProcAddress   (const char*);
  fromdll i32        __stdcall wglMakeCurrent      (void*, void*);
  fromdll void       __stdcall wglDeleteContext    (void*);
  fromdll void*      __stdcall GetDC               (void*);
  fromdll void       __stdcall ReleaseDC           (void*, void*);
  fromdll i32        __stdcall ChoosePixelFormat   (void*, PixelFormatDescriptor*);
  fromdll i32        __stdcall DescribePixelFormat (void*, i32, u32, PixelFormatDescriptor*);
  fromdll i32        __stdcall SetPixelFormat      (void*, i32, PixelFormatDescriptor*);
  fromdll void*      __stdcall wglCreateContext    (void*);
  fromdll iptr       __stdcall DefWindowProcW      (void*, u32, uptr, iptr);
  fromdll void*      __stdcall GetModuleHandleW    (const wchar_t*);
  fromdll u16        __stdcall RegisterClassExW    (WndClassExW*);
  fromdll i32        __stdcall UnregisterClassW    (const wchar_t*, void*);
  fromdll void*      __stdcall CreateWindowExW     (u32, const wchar_t*, const wchar_t*, u32, i32, i32, i32, i32, void*, void*, void*, void*);
  fromdll i32        __stdcall DestroyWindow       (void*);
}

namespace Co
{
  static int context_attribs [] = {
    WGL_CONTEXT_MAJOR_VERSION, opengl_major,
    WGL_CONTEXT_MINOR_VERSION, opengl_minor,
    WGL_CONTEXT_PROFILE_MASK,  WGL_CONTEXT_CORE_PROFILE_BIT, // Ignored for 3.1 and earlier
  #ifndef NDEBUG
    WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
  #endif
    0, 0
  };

  //
  // Constructor
  //
  GLRenderer::GLRenderer (Log& log, WorkQueue& queue, const Clock& clock) :
    log   (log),
    queue (queue),
    clock (clock)
  {
    target    = 0;
    shared_dc = 0;
    format    = 0;
    shared_rc = 0;
    wglCreateContextAttribs = 0;

    geoms.reserve     (5000);
    meshes.reserve    (5000);
    materials.reserve (5000);
    labels_2d.reserve (2000);
    labels_3d.reserve (2000);
    rects.reserve     (5000);
    lights.reserve    (8);
  }

  //
  // = init ============================================================================================================
  // This is an absolute clusterfuck, mainly because WGL was designed by lunatics.
  // 1.  Create dummy window
  // 2.  Set any old OpenGL pixel format on this window
  // 3.  Create a context on this window, and make it current
  // 4.  Grab the extension functions we need to do everything over properly
  // 5.  Set a decent OpenGL pixel format on the target window
  // 6.  Create our shared DC and RC on the target window
  // 7.  Make the shared RC current temporarily
  // 8.  Cleanup dummy objects no sooner than at this point
  // 9.  Initialize GLEW, using new RC
  // 10. Un-current the shared RC
  //
  void GLRenderer::init (void* new_target) try
  {
    if (!new_target)
      throw std::invalid_argument ("Renderer::init - Null pointer");

    if (target)
      throw std::logic_error ("Renderer::init - Renderer already initialized");

    target = new_target;
    
    log () << "- GLRenderer initializing\n";

    // BULLSHIT OPENGL INITIALIZATION
    
    // Make a dummy window for our extension-grabbing crap
    WndClassExW dummy_class = {
      sizeof (dummy_class),
      0,
      DefWindowProcW,
      0, 0,
      GetModuleHandleW (0),
      0,
      0,
      0,
      0,
      L"Co::GLRenderer-dummy",
      0
    };

    u16 dummy_atom = RegisterClassExW (&dummy_class);
    if (!dummy_atom)
      throw Rk::WinError ("Error registering dummy window class");
    auto class_guard = Rk::guard (
      [&dummy_class] { UnregisterClassW (dummy_class.class_name, dummy_class.instance); }
    );

    void* dummy_window = CreateWindowExW (
      0,
      (const wchar_t*) uptr (dummy_atom),
      L"Cobalt OpenGL renderer dummy",
      0,
      100, 100,
      100, 100,
      0, 0,
      GetModuleHandleW (0),
      0
    );
    if (!dummy_window)
      throw Rk::WinError ("Error creating dummy window");
    auto window_guard = Rk::guard (
      [dummy_window] { DestroyWindow (dummy_window); }
    );

    // Dummy DC
    void* dummy_dc = GetDC (dummy_window);
    if (!dummy_dc)
      throw Rk::WinError ("Error retrieving dummy device context");
    // Dummy DC is unguarded, since a DC is bound to its window

    // Dummy PFD
    PixelFormatDescriptor dummy_pfd = { 0 };
    dummy_pfd.size       = sizeof (PixelFormatDescriptor);
    dummy_pfd.version    = 1;
    dummy_pfd.flags      = pfd_support_opengl | pfd_draw_to_window;
    dummy_pfd.pixel_type = pfd_type_rgba;
    dummy_pfd.color_bits = 24;
    dummy_pfd.depth_bits = 16;
    dummy_pfd.layer_type = pfd_main_plane;

    format = ChoosePixelFormat (dummy_dc, &dummy_pfd);
    if (!format)
      throw Rk::WinError ("No appropriate dummy pixel format supported");

    i32 ok = SetPixelFormat (dummy_dc, format, &dummy_pfd);
    if (!ok)
      throw Rk::WinError ("Error setting dummy pixel format");
    
    // Dummy context
    void* dummy_rc = wglCreateContext (dummy_dc);
    if (!dummy_rc)
      throw std::runtime_error ("Error creating dummy render context");
    // Dummy context unguarded; again, bound to the window

    ok = wglMakeCurrent (dummy_dc, dummy_rc);
    if (!ok)
      throw std::runtime_error ("Error making dummy render context current");

    // Grab extensions
    typedef i32 (__stdcall *WGLCPF) (void*, const i32*, const float*, u32, i32*, u32*);
    auto wglChoosePixelFormat = (WGLCPF) wglGetProcAddress ("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormat)
      throw std::runtime_error ("wglChoosePixelFormat not supported");

    wglCreateContextAttribs = (WGLCCA) wglGetProcAddress ("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribs)
      throw std::runtime_error ("wglCreateContextAttribsARB not supported");
    
    // Got our extensions. Now do EVERYTHING OVER with our real window and our real functions.

    // Shared DC
    shared_dc = GetDC (target);
    if (!shared_dc)
      throw Rk::WinError ("Error retrieving shared device context");

    // Decent pixel format
    i32 pixel_format_int_attribs [] = {
      WGL_DRAW_TO_WINDOW, true,
      WGL_SUPPORT_OPENGL, true,
      WGL_ACCELERATION,   WGL_FULL_ACCELERATION,
      WGL_COLOR_BITS,     24,
      WGL_ALPHA_BITS,     8,
      WGL_DEPTH_BITS,     16,
      WGL_DOUBLE_BUFFER,  true,
      WGL_SAMPLE_BUFFERS, true,
      WGL_SAMPLES,        4,
      0, 0
    };

    u32 format_count;
    ok = wglChoosePixelFormat (shared_dc, pixel_format_int_attribs, nullptr, 1, &format, &format_count);
    if (!ok || format_count == 0)
      throw std::runtime_error ("No appropriate pixel format available");

    ok = SetPixelFormat (shared_dc, format, &dummy_pfd); // PFD pointer is irrelevant, but necessary. Fucking WinAPI.
    if (!ok)
      throw Rk::WinError ("Error setting pixel format");
    
    // Shared context
    shared_rc = wglCreateContextAttribs (shared_dc, 0, context_attribs);
    u32 code = Rk::ExceptionPrivate::GetLastError ();
    if (!shared_rc)
      throw std::runtime_error ("Error creating shared render context");
    
    // Temporarily make shared context current, to initialize GLEW
    dummy_dc = GetDC (target);
    if (!dummy_dc)
      throw Rk::WinError ("Error retrieving second dummy device context");

    ok = wglMakeCurrent (dummy_dc, shared_rc);
    if (!ok)
    {
      ReleaseDC (target, dummy_dc);
      throw Rk::WinError ("Error making shared context current");
    }
    auto current_guard = Rk::guard (
      [] { wglMakeCurrent (0, 0); }
    );

    // Initialize GLEW
    glewExperimental = true; // what in sam hell
    bool fail = glewInit () != GLEW_OK;
    if (fail)
      throw std::runtime_error ("Error initializing GLEW");
    
    // Done with OpenGL, at last.
    current_guard.relieve ();

    true_context = create_context_impl ();
    
    geom_program   = GeomProgram::create ();
    rect_program   = RectProgram::create ();
    skybox_program = SkyboxProgram::create ();
    
    if (wglewIsSupported ("WGL_EXT_swap_control"))
    {
      auto wglSwapInterval = (i32 (__stdcall*) (i32)) wglGetProcAddress ("wglSwapIntervalEXT");
      wglSwapInterval (0);
    }

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE);
    glEnable (GL_FRAMEBUFFER_SRGB);
    glEnable (GL_MULTISAMPLE);
    check_gl ("glEnable");
  }
  catch (...)
  {
    cleanup ();
    throw;
  }
  
  void GLRenderer::cleanup ()
  {
    true_context.reset ();

    if (shared_rc)
    {
      wglDeleteContext (shared_rc);
      shared_rc = 0;
    }
    
    if (shared_dc)
    {
      ReleaseDC (target, shared_dc);
      shared_dc = 0;
    }

    target  = 0;
    format  = 0;
    wglCreateContextAttribs = 0;
  }

  RenderContext& GLRenderer::context ()
  {
    return *true_context;
  }

  //
  // Context creation
  //
  GLContext::Ptr GLRenderer::create_context_impl ()
  {
    //log () << "- Creating render context on GLRenderer\n";
    return GLContext::Ptr (
      new GLContext (device_mutex, wglCreateContextAttribs, shared_dc, shared_rc, target, context_attribs)
    );
  }

  RenderContext::Ptr GLRenderer::create_context ()
  {
    return create_context_impl ();
  }

  //
  // = Rendering =======================================================================================================
  //
  void GLRenderer::render_geoms (mat4f world_to_clip)
  {
    glEnable (GL_DEPTH_TEST);
    //glDisable (GL_CULL_FACE);

    auto cur_mesh = meshes.begin    ();
    auto cur_mat  = materials.begin ();

    GLCompilation::Ptr prev_comp = nullptr;

    for (auto geom = geoms.begin (); geom != geoms.end (); geom++)
    {
      if (!geom -> comp)
      {
        cur_mesh += geom -> mesh_count;
        cur_mat  += geom -> material_count;
        continue;
      }

      if (geom -> comp != prev_comp)
      {
        if (prev_comp)
          prev_comp -> done ();
        bool ok = geom -> comp -> use ();
        prev_comp = geom -> comp;
      }

      static const GLenum index_types [4] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
      static const uptr   index_sizes [4] = { 0, 1, 2, 4 };
      
      IndexType comp_index_type = geom -> comp -> get_index_type ();
      GLenum index_type = index_types [comp_index_type];
      uptr   index_size = index_sizes [comp_index_type];

      geom_program -> set_model_to_clip (
        world_to_clip *
        Rk::affine_xform (
          geom -> spat.position,
          geom -> spat.orientation
        )
      );
      
      auto end_mesh = cur_mesh + geom -> mesh_count;
      while (cur_mesh != end_mesh)
      {
        const Mesh& mesh = *cur_mesh++;

        auto tex = static_cast <GLTexImage*> ((cur_mat + mesh.material) -> diffuse_tex.get ());
        if (tex)
          tex -> bind (geom_program -> texunit_diffuse);
        else
          GLTexImage::unbind (geom_program -> texunit_diffuse);

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
          uptr offset = index_size * mesh.base_index + geom -> comp -> index_base ();
          u32 eb = (u32) geom -> comp -> element_base ();
          glDrawElementsBaseVertex (
            prim_type,
            mesh.element_count,
            index_type,
            (void*) offset,
            mesh.base_element + eb
          );
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

  void GLRenderer::render_labels_3d ()
  {

  }

  void GLRenderer::render_labels_2d ()
  {
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (auto label = labels_2d.begin (); label != labels_2d.end (); label++)
    {
      if (label -> tex)
        label -> tex -> bind (rect_program -> texunit_tex);
      else
        GLTexImage::unbind (rect_program -> texunit_tex);

      rect_program -> set_linear_colour (label -> linear_colour);
      rect_program -> set_const_colour  (label -> const_colour);
      rect_program -> set_transform     (label -> spat);
      uptr offset = label -> first * 32;

      //glDrawArraysInstancedBaseInstance ( // DAMNIT

      glVertexAttribIPointer (rect_program -> attrib_rect, 4, GL_INT, 32, (void*) uptr (offset + 0));
      check_gl ("glVertexAttribIPointer");

      glVertexAttribIPointer (rect_program -> attrib_tcoords, 4, GL_INT, 32, (void*) uptr (offset + 16));
      check_gl ("glVertexAttribIPointer");

      glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, label -> count);
      check_gl ("glDrawArraysInstanced");
    }

    glDisable (GL_BLEND);
  }

  void GLRenderer::render_frame ()
  {
    // Setup for drawing
    auto back = pow (v3f (0.00f, 0.26f, 0.51f), 2.2f);
    glClearColor (back.x, back.y, back.z, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl ("glClear");

    // Transformations
    auto world_to_eye = Rk::world_to_eye_xform (
      camera_spat.position,
      camera_spat.orientation
    );
    
    float aspect = float (width) / float (height);

    auto eye_to_clip = Rk::eye_to_clip_xform (
      camera_fov,
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
        camera_spat.orientation
      );

      auto sky_to_clip = eye_to_clip * sky_to_eye;

      skybox_tex -> bind (skybox_program -> texunit_cube);
      skybox_program -> render (sky_to_clip, skybox_colour, skybox_alpha);
    }
    
    // Render point geometries
    geom_program -> use ();
    render_geoms (world_to_clip);
    geom_program -> done ();

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
    rect_program -> use ();
    rect_program -> set_ui_to_clip (ui_to_clip);
    rect_program -> upload_rects (rects.data (), rects.size ());

    //render_labels_3d ();
    render_labels_2d ();

    rect_program -> done ();

    true_context -> present ();

    geoms.clear ();
    meshes.clear ();
    materials.clear ();
    labels_2d.clear ();
    labels_3d.clear ();
    rects.clear ();
    lights.clear ();

    skybox_tex = nullptr;
  }

  //
  // = Frame building ==================================================================================================
  //
  void GLRenderer::begin_point_geom (GeomCompilation::Ptr comp, Spatial spat)
  {
    geoms.push_back (
      PointGeom (
        std::static_pointer_cast <GLCompilation> (comp),
        spat
      )
    );
  }

  void GLRenderer::add_meshes (const Mesh* begin, const Mesh* end)
  {
    if (geoms.empty ())
      throw std::logic_error ("No current geom");
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;
    
    meshes.insert (meshes.end (), begin, end);
    geoms.back ().mesh_count += (u32) size;
  }

  void GLRenderer::add_materials (const Material* begin, const Material* end)
  {
    if (geoms.empty ())
      throw std::logic_error ("No current geom");
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;

    materials.insert (materials.end (), begin, end);
    geoms.back ().material_count += (u32) size;
  }

  void GLRenderer::end ()
  {

  }

  void GLRenderer::add_label (
    TexImage::Ptr  texture,
      const TexRect* begin,
      const TexRect* end,
      Spatial2D      spat,
      v4f            linear_colour,
      v4f            const_colour)
  {
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;

    labels_2d.push_back (
      Label2D (
        std::static_pointer_cast <GLTexImage> (texture),
        (u32) rects.size (),
        (u32) size,
        spat,
        linear_colour,
        const_colour
      )
    );
    rects.insert (rects.end (), begin, end);
  }
  
  void GLRenderer::add_label (
    TexImage::Ptr  texture,
      const TexRect* begin,
      const TexRect* end,
      Spatial        spat,
      v4f            linear_colour,
      v4f            const_colour)
  {
    auto size = Rk::check_range (begin, end);
    if (!size)
      return;

    labels_3d.push_back (
      Label3D (
        std::static_pointer_cast <GLTexImage> (texture),
        (u32) rects.size (),
        (u32) size,
        spat,
        linear_colour,
        const_colour
      )
    );
    rects.insert (rects.end (), begin, end);
  }

  void GLRenderer::add_lights (const Light* begin, const Light* end)
  {
    if (!Rk::check_range (begin, end))
      return;

    lights.insert (lights.end (), begin, end);
  }
  
  void GLRenderer::set_skybox (TexImage::Ptr cube, v3f colour, float alpha)
  {
    skybox_tex    = std::static_pointer_cast <GLTexImage> (cube);
    skybox_colour = colour;
    skybox_alpha  = alpha;
  }

  void GLRenderer::set_camera (Spatial spat, float fov, float near, float far)
  {
    camera_spat = spat;
    camera_fov  = fov;
    camera_near = near;
    camera_far  = far;
  }

  //
  // = Module ==========================================================================================================
  //
  class Root :
    public RendererRoot
  {
    virtual Renderer::Ptr create_renderer (Log& log, WorkQueue& queue, const Clock& clock)
    {
      return std::make_shared <GLRenderer> (log, queue, clock);
    }

  };

  RK_MODULE (Root);

} // namespace Co
