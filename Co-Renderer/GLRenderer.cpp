//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLRenderer.hpp"

// Uses
#include <Co/IxGeomCompilation.hpp>
#include <Co/Clock.hpp>

#include <Rk/Expose.hpp>
#include <Rk/File.hpp>

#include "GeomProgram.hpp"
#include "RectProgram.hpp"
#include "GLTexImage.hpp"
#include "GL.hpp"

//
// = WGL stuff =========================================================================================================
//
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
  pfd_support_opengl      = 0x00000020,
  pfd_draw_to_window      = 0x00000004,
  pfd_doublebuffer        = 0x00000001,
  pfd_support_gdi         = 0x00000010,
  pfd_generic_format      = 0x00000040,
  pfd_generic_accelerated = 0x00001000,
  pfd_swap_copy           = 0x00000400,
  pfd_need_palette        = 0x00000080,
  pfd_need_system_palette = 0x00000100,
      
  pfd_type_rgba = 0,
      
  pfd_main_plane = 0
};

typedef void  (__stdcall *GLFunction) ();

extern "C"
{
  __declspec(dllimport) GLFunction __stdcall wglGetProcAddress   (const char*);
  __declspec(dllimport) i32        __stdcall wglMakeCurrent      (void*, void*);
  __declspec(dllimport) void       __stdcall wglDeleteContext    (void*);
  __declspec(dllimport) void*      __stdcall GetDC               (void*);
  __declspec(dllimport) void       __stdcall ReleaseDC           (void*, void*);
  __declspec(dllimport) i32        __stdcall ChoosePixelFormat   (void*, PixelFormatDescriptor*);
  __declspec(dllimport) i32        __stdcall DescribePixelFormat (void*, i32, u32, PixelFormatDescriptor*);
  __declspec(dllimport) i32        __stdcall SetPixelFormat      (void*, i32, PixelFormatDescriptor*);
  __declspec(dllimport) void*      __stdcall wglCreateContext    (void*);
}

namespace Co
{
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
  // Destructor
  //
  GLRenderer::~GLRenderer ()
  {
    cleanup ();
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
    log << "- GLRenderer running\n";

    GLContext::Ptr context = create_context_impl ();

    // Set up shaders
    GeomProgram geom_program;
    RectProgram rect_program;

    SetThreadPriority (GetCurrentThread (), 31); // THREAD_PRIORITY_TIME_CRITICAL

    if (wglewIsSupported ("WGL_EXT_swap_control"))
    {
      auto wglSwapInterval = (i32 (__stdcall*) (i32)) wglGetProcAddress ("wglSwapIntervalEXT");
      wglSwapInterval (0);
    }

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE);

    uint frames = 0;
    GLFrame* frame = 0;
    
    float now = clock -> time () - 0.1f;
    //float prev = now;

    // The real loop
    for (;;)
    {
      /*if (now - prev > 0.1f)
        __asm nop;*/

      // Perhaps it's time to grab a new frame
      while (!frame || now >= frame -> time)
      {
        GLFrame* new_frame;
        
        ready_frames_mutex.lock ();
          bool got_frame = ready_frames.pop_front (new_frame);
        ready_frames_mutex.unlock ();
        
        if (got_frame)
        {
          if (frame)
          {
            auto lock = free_frames_mutex.get_lock ();
            free_frames.push_back (frame);
          }

          if (!new_frame)
            return;

          frame = new_frame;
        }
      }
      
      // Let's render it
      float alpha;

      if (now >= frame -> prev_time)
        alpha = (now - frame -> prev_time) / (frame -> time - frame -> prev_time);
      else
        alpha = 0.0f;

      float t = clock -> time ();

      frame -> render (alpha, geom_program, rect_program);
      
      float t2 = clock -> time ();

      if (t2 - t > 0.05f)
        __asm nop;

      context -> present ();
      frames++;

      float t3 = clock -> time ();

      //prev = now;
      now = t3 - 0.1f; //clock -> time () - 0.1f;

      if (t3 - t2 > 0.05f)
        __asm nop;
    }
  }
  catch (const std::exception& e)
  {
    log << "X " << e.what () << '\n';
  }
  catch (...)
  {
    log << "X Exception caught\n";
  }

  //
  // Initialization
  //
  bool GLRenderer::init (void* new_target, Clock* new_clock, Rk::IxLockedOutStreamImpl* new_logger) try
  {
    if (!new_target || !new_clock || !new_logger)
      throw Rk::Exception ("null pointer");

    if (target)
      throw Rk::Exception ("renderer already initialized");

    target = new_target;
    clock  = new_clock;
    log.set_impl (new_logger);

    log << "- GLRenderer initializing\n";

    shared_dc = GetDC (target);
    if (!shared_dc)
      throw Rk::Exception ("error retrieving shared device context");

    PixelFormatDescriptor pfd = { 0 };
    pfd.size       = sizeof (PixelFormatDescriptor);
    pfd.version    = 1;
    pfd.flags      = pfd_support_opengl | pfd_draw_to_window | pfd_doublebuffer;
    pfd.pixel_type = pfd_type_rgba;
    pfd.color_bits = 24;
    pfd.depth_bits = 16;
    pfd.layer_type = pfd_main_plane;

    format = ChoosePixelFormat (shared_dc, &pfd);
    if (!format)
      throw Rk::Exception ("no adequate pixel format supported");
    
    int ok = DescribePixelFormat (shared_dc, format, sizeof (pfd), &pfd);
    if (!ok)
      throw Rk::Exception ("DescribePixelFormat failed");
    
    enum
    {
      // These flags should always be present in a good PFD
      set_flags = pfd_support_opengl | pfd_draw_to_window | pfd_doublebuffer,

      // These flags should ONLY appear in rubbish, useless PFDs
      clear_flags = pfd_support_gdi | pfd_generic_format | pfd_generic_accelerated | pfd_swap_copy | pfd_need_palette |
        pfd_need_system_palette
    };
    
    if (
      (pfd.flags & set_flags  ) != set_flags ||
      (pfd.flags & clear_flags) != 0         ||
      pfd.pixel_type != pfd_type_rgba        ||
      pfd.color_bits < 24                    ||
      pfd.depth_bits < 16                    ||
      pfd.layer_type != pfd_main_plane)
    {
      throw Rk::Exception ("ChoosePixelFormat chose bogus format");
    }
    
    ok = SetPixelFormat (shared_dc, format, &pfd);
    if (!ok)
      throw Rk::Exception ("error setting pixel format");
    
    // Get wglCreateContextAttribs
    void* temp_rc = wglCreateContext (shared_dc);
    if (!temp_rc)
      throw Rk::Exception ("error creating temporary render context");
    
    void* temp_dc = GetDC (target);
    if (!temp_dc)
    {
      wglDeleteContext (temp_rc);
      throw Rk::Exception ("error retrieving temporary device context");
    }

    ok = wglMakeCurrent (temp_dc, temp_rc);
    if (!ok)
    {
      wglDeleteContext (temp_rc);
      ReleaseDC (target, temp_dc);
      throw Rk::Exception ("error making temporary render context current");
    }
    
    wglCreateContextAttribs = (WGLCCA) wglGetProcAddress ("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribs)
    {
      wglMakeCurrent (0, 0);
      throw Rk::Exception ("wglCreateContextAttribsARB not supported");
    }
    
    bool fail = glewInit () != GLEW_OK;
    if (fail)
    {
      wglMakeCurrent (0, 0);
      throw Rk::Exception ("error initializing GLEW");
    }
    
    int attribs [] = {
      WGL_CONTEXT_MAJOR_VERSION, opengl_major,
      WGL_CONTEXT_MINOR_VERSION, opengl_minor,
      WGL_CONTEXT_PROFILE_MASK,  WGL_CONTEXT_CORE_PROFILE_BIT, // Ignored for 3.1 and earlier
    #ifndef NDEBUG
      WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
    #endif
      0, 0
    };
    
    shared_rc = wglCreateContextAttribs (shared_dc, 0, attribs);

    wglMakeCurrent (0, 0);

    if (!shared_rc)
      throw Rk::Exception ("error creating shared render context");

    prepping_frame = 0;

    return true;
  }
  catch (const std::exception& e)
  {
    if (log)
      log << "X " << e.what () << '\n';
    cleanup ();
    return false;
  }
  catch (...)
  {
    if (log)
      log << "X Exception caught\n";
    cleanup ();
    return false;
  }

  //
  // Cleanup
  //
  void GLRenderer::cleanup ()
  {
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

    log    = nil;
    clock  = 0;
    target = 0;
    format = 0;
    wglCreateContextAttribs = 0;
    prepping_frame = 0;
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
  GLContext* GLRenderer::create_context_impl ()
  {
    log << "- Creating render context on GLRenderer\n";
    return new GLContext (device_mutex, wglCreateContextAttribs, shared_dc, shared_rc, target);
  }

  IxRenderContext* GLRenderer::create_context ()
  {
    return create_context_impl ();
  }

  //
  // Rendering control
  //
  void GLRenderer::start ()
  {
    if (thread)
      throw Rk::Exception ("Co-GLRenderer: IxRenderer::start - renderer already running");

    ready_frames.clear ();
    free_frames.clear ();

    for (uint i = 0; i != frame_count; i++)
    {
      frames [i].id = i + 1;
      free_frames.push_back (&frames [i]);
    }

    log << "- GLRenderer starting\n";

    thread.execute (
      [this] { loop (); }
    );
  }

  void GLRenderer::stop ()
  {
    if (!thread)
      return;

    if (prepping_frame)
      throw Rk::Exception ("Co-GLRenderer: IxRenderer::stop - attempt to stop renderer while preparing frame");

    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (0);
    lock = nil;
    
    log << "- GLRenderer stopping\n";

    thread.join ();
  }
  
  //
  // Frame specification
  //
  IxFrame* GLRenderer::begin_frame (float prev_time, float current_time, u32& old_frame_id, u32& new_frame_id)
  {
    if (prepping_frame)
      throw Rk::Exception ("Co-GLRenderer: IxRenderer::begin_frame - current frame must be submitted before requesting a new one");

    while (!prepping_frame)
    {
      auto lock = free_frames_mutex.get_lock ();
      free_frames.pop_front (prepping_frame);
    }
    
    old_frame_id = prepping_frame -> reset (prev_time, current_time, frame_count, new_frame_id);

    return prepping_frame;
  }

  void GLRenderer::submit_frame (IxFrame* frame)
  {
    auto gl_frame = static_cast <GLFrame*> (frame);

    if (prepping_frame != gl_frame)
      throw Rk::Exception ("Co-GLRenderer: IxRenderer::submit_frame - submitted frame does not match with tracked current frame");

    prepping_frame -> set_size (width, height);

    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (gl_frame);

    prepping_frame = 0;
  }

  void GLRenderer::add_garbage_vao (u32 vao)
  {
    if (!prepping_frame)
      throw Rk::Exception ("Co-GLRenderer: GLRenderer::add_garbage_vao - VAO disposal request with no current frame");
    prepping_frame -> garbage_vaos [prepping_frame -> garbage_vao_back_index++] = vao;
  }

  //
  // Interface exposure
  //
  void GLRenderer::expose (u64 ixid, void** result)
  {
    Rk::expose <IxRenderer>     (this, ixid, result);
    Rk::expose <IxRenderDevice> (this, ixid, result);
  }

} // namespace Co
