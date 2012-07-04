//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLRenderer.hpp"

// Uses
#include <Co/GeomCompilation.hpp>
#include <Co/Clock.hpp>

#include <Rk/Module.hpp>
#include <Rk/Guard.hpp>
#include <Rk/File.hpp>

#include "SkyboxProgram.hpp"
#include "GeomProgram.hpp"
#include "RectProgram.hpp"
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
  // Instance
  //
  GLRenderer GLRenderer::instance;

  //
  // Constructor
  //
  GLRenderer::GLRenderer ()
  {
    target    = 0;
    clock     = 0;
    shared_dc = 0;
    format    = 0;
    shared_rc = 0;
    wglCreateContextAttribs = 0;
  }

  //
  // Rendering loop
  //
  extern "C"
  {
    __declspec(dllimport) void* __stdcall GetCurrentThread  ();
    __declspec(dllimport) i32   __stdcall SetThreadPriority (void*, u32);
  }

  void GLRenderer::loop () try
  {
    log () << "- GLRenderer running\n";

    auto context = create_context_impl ();

    // Set up shaders
    SkyboxProgram skybox_program;
    GeomProgram   geom_program;
    RectProgram   rect_program;

    //SetThreadPriority (GetCurrentThread (), 31); // THREAD_PRIORITY_TIME_CRITICAL

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

    uint frames = 0;
    GLFrame* frame = 0;
    
    float now;
    
    // The real loop
    for (;;)
    {
      now = clock -> time () - 0.050f;

      // Perhaps it's time to grab a new frame
      while (!frame || now >= frame -> time)
      {
        // Is there a new frame?
        auto lock = ready_frames_mutex.get_lock ();
        if (ready_frames.empty ())
          continue; // Keep trying

        // Grab the new frame
        auto new_frame = ready_frames.pop_front ();
        lock = nil;

        // Do we have an old frame?
        if (frame)
        {
          // Recycle it
          auto lock = free_frames_mutex.get_lock ();
          free_frames.push_back (frame);
        }

        // Null frame indicates renderer shutdown
        if (!new_frame)
          return;

        // Use our new frame for rendering
        frame = new_frame;
      }
      
      // Animate smoothly
      float alpha = 0.0f;
      if (now >= frame -> prev_time)
        alpha = (now - frame -> prev_time) / (frame -> time - frame -> prev_time);
      
      if (alpha > 1.0f || alpha < 0.0f)
        __asm nop;

      frame -> render (alpha, skybox_program, geom_program, rect_program);
      
      context -> present ();
      frames++;
    }
  }
  catch (const std::exception& e)
  {
    log () << "X Exception caught in renderer:\n"
           << "X " << e.what () << '\n';
  }
  catch (...)
  {
    log () << "X Exception caught in renderer\n";
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
  void GLRenderer::init (void* new_target, const Clock& new_clock, Log& new_logger) try
  {
    if (!new_target)
      throw std::invalid_argument ("Renderer::init - Null pointer");

    if (target || thread)
      throw std::logic_error ("Renderer::init - Renderer already initialized");

    target = new_target;
    clock  = &new_clock;
    log_ptr = &new_logger;

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
    auto curent_guard = Rk::guard (
      [] { wglMakeCurrent (0, 0); }
    );

    // Initialize GLEW
    bool fail = glewInit () != GLEW_OK;
    if (fail)
      throw std::runtime_error ("Error initializing GLEW");
    
    // Done with OpenGL, at last. Initialize frame queues and start the rendering thread.
    ready_frames.clear ();
    free_frames.clear ();

    for (uint i = 0; i != frame_count; i++)
      free_frames.push_back (&frames [i]);

    log () << "- GLRenderer starting\n";

    thread.execute (
      [this] { loop (); }
    );
  }
  catch (...)
  {
    cleanup ();
  }

  void GLRenderer::stop ()
  {
    if (!thread)
      return;

    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (nullptr);
    lock = nil;
    
    log () << "- GLRenderer stopping\n";

    thread.join ();
  }
  
  void GLRenderer::cleanup ()
  {
    stop ();

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

    log_ptr = 0;
    clock   = 0;
    target  = 0;
    format  = 0;
    wglCreateContextAttribs = 0;
  }

  //
  // Device size
  //
  void GLRenderer::set_size (uint new_width, uint new_height)
  {
    width  = new_width;
    height = new_height;
  }

  //
  // Context creation
  //
  GLContext::Ptr GLRenderer::create_context_impl ()
  {
    log () << "- Creating render context on GLRenderer\n";
    return GLContext::Ptr (
      new GLContext (device_mutex, wglCreateContextAttribs, shared_dc, shared_rc, target, context_attribs)
    );
  }

  RenderContext::Ptr GLRenderer::create_context ()
  {
    return create_context_impl ();
  }
  
  //
  // Frame specification
  //
  Frame* GLRenderer::begin_frame (float prev_time, float current_time)
  {
    if (!thread)
      throw std::logic_error ("Renderer::begin_frame - Renderer not running");

    GLFrame* frame = nullptr;
    while (!frame)
    {
      auto lock = free_frames_mutex.get_lock ();
      if (!free_frames.empty ())
        frame = free_frames.pop_front ();
    }
    
    frame -> reset (prev_time, current_time);
    return frame;
  }

  void GLRenderer::submit_frame (GLFrame* frame)
  {
    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (frame);
  }

  std::shared_ptr <GLRenderer> GLRenderer::create ()
  {
    return std::shared_ptr <GLRenderer> (
      &instance,
      [] (GLRenderer* p) { p -> cleanup (); }
    );
  }

  RK_MODULE_FACTORY (GLRenderer);

} // namespace Co
