//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxRenderDevice.hpp>
#include <Co/IxRenderer.hpp>
#include <Co/Frame.hpp>

// Uses
#include <Co/IxGeomBuffer.hpp>
#include <Co/IxTexture.hpp>
#include <Co/Clock.hpp>

#include <Rk/FixedQueue.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Expose.hpp>
#include <Rk/Thread.hpp>
#include <Rk/Mutex.hpp>

#include <gl/glew.h>

#include "GLCompilation.hpp"
#include "GLTexture.hpp"
#include "GLBuffer.hpp"

//
// = WGL stuff =========================================================================================================
//
namespace
{
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
  
  enum
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
  typedef void* (__stdcall *WGLCCA)     (void*, void*, const int*);

  extern "C"
  {
    __declspec(dllimport) GLFunction __stdcall wglGetProcAddress   (const char*);
    __declspec(dllimport) void*      __stdcall GetDC               (void*);
    __declspec(dllimport) i32        __stdcall ChoosePixelFormat   (void*, PixelFormatDescriptor*);
    __declspec(dllimport) i32        __stdcall DescribePixelFormat (void*, i32, u32, PixelFormatDescriptor*);
    __declspec(dllimport) i32        __stdcall SetPixelFormat      (void*, i32, PixelFormatDescriptor*);
    __declspec(dllimport) void*      __stdcall wglCreateContext    (void*);
    __declspec(dllimport) i32        __stdcall wglMakeCurrent      (void*, void*);
    __declspec(dllimport) void       __stdcall wglDeleteContext    (void*);
    __declspec(dllimport) void       __stdcall ReleaseDC           (void*, void*);
    __declspec(dllimport) void       __stdcall SwapBuffers         (void*);
  }

  enum
  {
    WGL_CONTEXT_MAJOR_VERSION    = 0x2091,
    WGL_CONTEXT_MINOR_VERSION    = 0x2092,
    WGL_CONTEXT_PROFILE_MASK     = 0x9126,
    WGL_CONTEXT_FLAGS            = 0x2094,
    WGL_CONTEXT_DEBUG_BIT        = 0x0001,
    WGL_CONTEXT_CORE_PROFILE_BIT = 0x00000001
  };

}

namespace Co
{
namespace
{
  //
  // = OpenGL version ==================================================================================================
  //
  enum
  {
    opengl_major = 3,
    opengl_minor = 3
  };

  //
  // = GLContext =======================================================================================================
  //
  class GLContext :
    public IxRenderContext
  {
    Rk::Mutex& mutex;
    void*      dc;
    void*      rc;

  public:
    typedef Rk::IxUniquePtr <GLContext> Ptr;

    GLContext (Rk::Mutex& device_mutex, WGLCCA wglCCA, void* shared_dc, void* shared_rc, void* target);

    void present ();

    virtual IxGeomBuffer*      create_buffer      (uptr size, const void* data);
    virtual IxGeomCompilation* create_compilation (const GeomAttrib*, uint attrib_count, IxGeomBuffer* elements, IxGeomBuffer* indices);
    virtual IxTexture*         create_texture     (uint level_count, bool wrap);

    ~GLContext ();
    virtual void destroy ();

  }; // class GLContext
  
  //
  // Constructor
  //
  GLContext::GLContext (Rk::Mutex& device_mutex, WGLCCA wglCCA, void* shared_dc, void* shared_rc, void* target) try :
    mutex (device_mutex)
  {
    auto lock = mutex.get_lock ();
      
    int attribs [] = {
      WGL_CONTEXT_MAJOR_VERSION, opengl_major,
      WGL_CONTEXT_MINOR_VERSION, opengl_minor,
      WGL_CONTEXT_PROFILE_MASK,  WGL_CONTEXT_CORE_PROFILE_BIT, // Ignored for 3.1 and earlier
      WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
      0,                         0
    };
      
    rc = wglCCA (shared_dc, shared_rc, attribs);
    if (!rc)
      throw Rk::Exception ("Error creating render context");
      
    dc = GetDC (target);
    if (!dc)
      throw Rk::Exception ("Error retrieving device context");
      
    bool ok = wglMakeCurrent (dc, rc);
    if (!ok)
      throw Rk::Exception ("Error making render context current");
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLContext::GLContext");

    if (dc)
      ReleaseDC (target, dc);

    if (rc)
      wglDeleteContext (rc);
  }

  //
  // Present
  //
  void GLContext::present ()
  {
    glFlush ();
    SwapBuffers (dc);
  }

  //
  // Buffer creation
  //
  IxGeomBuffer* GLContext::create_buffer (uptr size, const void* data)
  {
    return new GLBuffer (size, data);
  }

  //
  // Compilation creation
  //
  IxGeomCompilation* GLContext::create_compilation (
    const GeomAttrib* attribs, uint attrib_count, IxGeomBuffer* elements, IxGeomBuffer* indices)
  {
    return new GLCompilation (attribs, attrib_count, static_cast <GLBuffer*> (elements), static_cast <GLBuffer*> (indices));
  }

  //
  // Texture creation
  //
  IxTexture* GLContext::create_texture (uint level_count, bool wrap)
  {
    return new GLTexture (level_count, wrap);
  }

  //
  // Destructors
  //
  GLContext::~GLContext ()
  {
    if (dc && rc)
    {
      auto lock = mutex.get_lock ();
      wglMakeCurrent (0, 0);
      wglDeleteContext (rc);
    }
  }

  void GLContext::destroy ()
  {
    delete this;
  }

  //
  // = GLFrame =========================================================================================================
  //
  class GLFrame :
    public Frame
  {
  public:
    uint id;
    float time, prev_time;

    void render (float alpha);
    
    u32 reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id);

  }; // class GLFrame

  void GLFrame::render (float alpha)
  {
    glClearColor (0.0f, 0.03f, 0.2f, 1.0f);
    glViewport (0, 0, width, height);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  }

  u32 GLFrame::reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id)
  {
    prev_time = new_prev_time;
    time      = new_current_time;

    point_geoms_back_index = 0;
    meshes_back_index      = 0;
    ui_rects_back_index    = 0;
    lights_back_index      = 0;
    materials_back_index   = 0;
    
    u32 old_id = id;
    new_id = (id += id_advance);
    return old_id;
  }

  //
  // = GLRenderer ======================================================================================================
  //
  class GLRenderer :
    public IxRenderer,
    public IxRenderDevice
  {
    // Frame queuing
    enum { frame_count = 10 };
    GLFrame frames [frame_count];
    Rk::FixedQueue <GLFrame*, frame_count> free_frames;
    Rk::Mutex                              free_frames_mutex;
    Rk::FixedQueue <GLFrame*, frame_count> ready_frames;
    Rk::Mutex                              ready_frames_mutex;

    // Render device
    void*      target;
    Clock*     clock;
    Rk::Thread thread;
    void*      shared_dc;
    int        format;
    void*      shared_rc;
    WGLCCA     wglCreateContextAttribs;
    Rk::Mutex  mutex;

    void       loop ();
    GLContext* create_context_impl ();

    virtual void             init           (void* new_target, Clock* new_clock);
    virtual IxRenderContext* create_context ();
    virtual void             start          ();
    virtual void             stop           ();
    virtual Frame*           begin_frame    (float prev_time, float current_time, u32& old_frame_id, u32& new_frame_id);
    virtual void             submit_frame   (Frame*);

  public:
    GLRenderer ();

    void expose (u64 ixid, void*& result);

  } renderer;

  //
  // Constructor
  //
  GLRenderer::GLRenderer ()
  {
    target = 0;
    clock  = 0;
    shared_dc = 0;
    format = 0;
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
    GLContext::Ptr context = create_context_impl ();

    // Set up shader
    // ...

    SetThreadPriority (GetCurrentThread (), 31); // THREAD_PRIORITY_TIME_CRITICAL

    uint frames = 0;
    GLFrame* frame = 0;

    // The real loop
    for (;;)
    {
      float now = clock -> time () - 0.1f;
      
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
      float alpha = 0.0f;
      if (now >= frame -> prev_time)
        alpha = (now - frame -> prev_time) / (frame -> time - frame -> prev_time);
      
      frame -> render (alpha);
      
      context -> present ();
      frames++;
    }
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLRenderer::loop");
    throw;
  }

  //
  // Initialization
  //
  void GLRenderer::init (void* new_target, Clock* new_clock) try
  {
    if (target)
      throw Rk::Violation ("Renderer already initialized");

    if (!new_target)
      throw Rk::Violation ("target is null");

    if (!new_clock)
      throw Rk::Violation ("clock is null");

    target = new_target;
    clock  = new_clock;
    
    shared_dc = GetDC (target);
    if (!shared_dc)
      throw Rk::Exception ("Error retrieving shared device context");

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
      throw Rk::Exception ("No adequate pixel format supported");
    
    int ok = DescribePixelFormat (shared_dc, format, sizeof (pfd), &pfd);
    if (!ok)
      throw Rk::Exception ("DescribePixelFormat () failed");
    
    enum
    {
      set_flags = pfd_support_opengl | pfd_draw_to_window | pfd_doublebuffer,
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
      throw Rk::Exception ("ChoosePixelFormat () chose bogus format");
    }
    
    ok = SetPixelFormat (shared_dc, format, &pfd);
    if (!ok)
      throw Rk::Exception ("Error setting pixel format");
    
    // Get wglCreateContextAttribs
    void* temp_rc = wglCreateContext (shared_dc);
    if (!temp_rc)
      throw Rk::Exception ("Error creating temporary render context");
    
    void* temp_dc = GetDC (target);
    if (!temp_dc)
      throw Rk::Exception ("Error retrieving temporary device context");
    
    ok = wglMakeCurrent (temp_dc, temp_rc);
    if (!ok)
      throw Rk::Exception ("Error making temporary render context current");
    
    wglCreateContextAttribs = (WGLCCA) wglGetProcAddress ("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribs)
      throw Rk::Exception ("wglCreateContextAttribsARB () not supported");
    
    bool fail = glewInit ();
    if (fail)
      throw Rk::Exception ("Error initializing GLEW");
    
    int attribs [] = {
      WGL_CONTEXT_MAJOR_VERSION, opengl_major,
      WGL_CONTEXT_MINOR_VERSION, opengl_minor,
      WGL_CONTEXT_PROFILE_MASK,  WGL_CONTEXT_CORE_PROFILE_BIT, // Ignored for 3.1 and earlier
      WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
      0, 0
    };
    
    shared_rc = wglCreateContextAttribs (shared_dc, 0, attribs);
    if (!shared_rc)
      throw Rk::Exception ("Error creating shared render context");
    
    wglMakeCurrent (0, 0);
  }
  catch (...)
  {
    Rk::log_frame ("Co::GLRenderer::init");
    throw;
  }

  //
  // Context creation
  //
  GLContext* GLRenderer::create_context_impl ()
  {
    return new GLContext (mutex, wglCreateContextAttribs, shared_dc, shared_rc, target);
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
    for (uint i = 0; i != frame_count; i++)
    {
      frames [i].id = i + 1;
      free_frames.push_back (&frames [i]);
    }
    
    thread.execute (
      [this] { loop (); }
    );
  }

  void GLRenderer::stop ()
  {
    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (0);
    lock = nil;
    
    thread.join ();
  }
  
  //
  // Frame specification
  //
  Frame* GLRenderer::begin_frame (float prev_time, float current_time, u32&  old_frame_id, u32& new_frame_id)
  {
    auto lock = free_frames_mutex.get_lock ();
    GLFrame* free_frame = 0;

    if (free_frames.pop_front (free_frame))
      old_frame_id = free_frame -> reset (prev_time, current_time, frame_count, new_frame_id);

    return free_frame;
  }

  void GLRenderer::submit_frame (Frame* frame)
  {
    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (static_cast <GLFrame*> (frame));
  }

  void GLRenderer::expose (u64 ixid, void*& result)
  {
    Rk::expose <IxRenderer>     (this, ixid, result);
    Rk::expose <IxRenderDevice> (this, ixid, result);
  }

} // namespace
  
  //
  // Module interface
  //
  extern "C" __declspec(dllexport) void* __cdecl ix_expose (u64 ixid)
  {
    void* result = 0;
    renderer.expose (ixid, result);
    return result;
  }

} // namespace Co
