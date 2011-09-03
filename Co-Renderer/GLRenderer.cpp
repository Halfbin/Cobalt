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

#include "GL.hpp"
#include "GLTexImage.hpp"

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
  // Shader init
  //
  u32 GLRenderer::load_shader (const char* path, uint type)
  {
    Rk::File file (path, Rk::File::open_read_existing);
    u32 size = u32 (file.size ());
    std::unique_ptr <char []> buffer (new char [size]);
    file.read (buffer.get (), size);
    
    u32 shader = glCreateShader (type);
    check_gl ("glCreateShader");

    const char* data = buffer.get ();
    u32 len = size;
    glShaderSource (shader, 1, &data, (const i32*) &len);
    check_gl ("glShaderSource");

    glCompileShader (shader);
    check_gl ("glCompileShader");

    glGetShaderiv (shader, GL_INFO_LOG_LENGTH, (i32*) &len);
    check_gl ("glGetShaderiv (GL_INFO_LOG_LENGTH)");

    if (len > 1)
    {
      if (len > size)
        buffer.reset (new char [len]);
      glGetShaderInfoLog (shader, len, (i32*) &len, buffer.get ());
      check_gl ("glGetShaderInfoLog");
      
      log () << "- Infolog from shader compiler:\n"
             << Rk::StringRef (buffer.get (), len - 1)
             << '\n';
    }

    int ok;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &ok);
    check_gl ("glGetShaderiv (GL_COMPILE_STATUS)");

    if (!ok)
      throw Rk::Exception ("Error compiling shader");

    return shader;
  }

  static void link_attrib (u32 program, const char* name, u32 index)
  {
    glBindAttribLocation (program, index, name);
    check_gl ("glBindAttribLocation");
  }

  static u32 link_uniform (u32 program, const char* name)
  {
    u32 uniform = glGetUniformLocation (program, name);
    check_gl ("glGetUniformLocation");
    if (uniform == 0xffffffff)
      throw Rk::Exception ("Uniform not present or inactive");
    return uniform;
  }

  void GLRenderer::load_program ()
  {
    log () << "- GLRenderer loading shaders\n";

    u32 vertex_shader   = load_shader ("../Common/Shaders/Main-Vertex.glsl",   GL_VERTEX_SHADER  );
    u32 fragment_shader = load_shader ("../Common/Shaders/Main-Fragment.glsl", GL_FRAGMENT_SHADER);

    program = glCreateProgram ();
    check_gl ("glCreateProgram");

    glAttachShader (program, vertex_shader);
    check_gl ("glAttachShader (vertex_shader)");

    glAttachShader (program, fragment_shader);
    check_gl ("glAttachShader (fragment_shader)");

    link_attrib  (program, "attrib_position", attrib_position);
    link_attrib  (program, "attrib_normal",   attrib_normal  );
    link_attrib  (program, "attrib_tcoords",  attrib_tcoords );

    glLinkProgram (program);
    check_gl ("glLinkProgram");

    u32 len;
    glGetProgramiv (program, GL_INFO_LOG_LENGTH, (i32*) &len);
    check_gl ("glGetProgramiv (GL_INFO_LOG_LENGTH)");

    if (len > 1)
    {
      std::unique_ptr <char []> buffer (new char [len]);
      glGetProgramInfoLog (program, len, (i32*) &len, buffer.get ());
      check_gl ("glGetProgramInfoLog");

      log () << "- Infolog from program linker:\n"
             << Rk::StringRef (buffer.get (), len - 1)
             << '\n';
    }

    int ok;
    glGetProgramiv (program, GL_LINK_STATUS, &ok);
    check_gl ("glGetProgramiv (GL_LINK_STATUS)");

    model_to_world = link_uniform (program, "model_to_world");
    world_to_clip  = link_uniform (program, "world_to_clip" );
    world_to_eye   = link_uniform (program, "world_to_eye"  );

    glUseProgram (program);
    check_gl ("glUseProgram");

    u32 tex_diffuse = link_uniform (program, "tex_diffuse");
    glUniform1i (tex_diffuse, texunit_diffuse);
    check_gl ("glUniform1i");

    if (!ok)
      throw Rk::Exception ("Error linking shader program");
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

    GLContext::Ptr context = create_context_impl ();

    // Set up shader
    if (!opengl_compat)
      load_program ();

    SetThreadPriority (GetCurrentThread (), 31); // THREAD_PRIORITY_TIME_CRITICAL

    if (wglewIsSupported ("WGL_EXT_swap_control"))
    {
      auto wglSwapInterval = (i32 (__stdcall*) (i32)) wglGetProcAddress ("wglSwapIntervalEXT");
      wglSwapInterval (1);
    }

    glEnable (GL_DEPTH_TEST);
    glEnable (GL_CULL_FACE);

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
      
      frame -> render (alpha, model_to_world, world_to_clip, world_to_eye);
      
      context -> present ();
      frames++;
    }
  }
  catch (const std::exception& e)
  {
    log () << e.what () << '\n'
           << "from Co::GLRenderer::loop\n";
  }
  catch (...)
  {
    log () << "Exception from Co::GLRenderer::loop\n";
  }

  //
  // Initialization
  //
  void GLRenderer::init (void* new_target, Clock& new_clock, Log& new_logger) try
  {
    Rk::require (!target, "Renderer already initialized");
    Rk::require (new_target != 0, "target is null");

    target = new_target;
    clock  = &new_clock;
    logger = &new_logger;

    log () << "- GLRenderer initializing\n";

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
    {
      wglDeleteContext (temp_rc);
      throw Rk::Exception ("Error retrieving temporary device context");
    }

    ok = wglMakeCurrent (temp_dc, temp_rc);
    if (!ok)
    {
      wglDeleteContext (temp_rc);
      ReleaseDC (target, temp_dc);
      throw Rk::Exception ("Error making temporary render context current");
    }
    
    wglCreateContextAttribs = (WGLCCA) wglGetProcAddress ("wglCreateContextAttribsARB");
    if (!wglCreateContextAttribs)
    {
      wglMakeCurrent (0, 0);
      throw Rk::Exception ("wglCreateContextAttribsARB () not supported");
    }
    
    bool fail = glewInit () != GLEW_OK;
    if (fail)
    {
      wglMakeCurrent (0, 0);
      throw Rk::Exception ("Error initializing GLEW");
    }
    
    int attribs [] = {
      WGL_CONTEXT_MAJOR_VERSION, opengl_major,
      WGL_CONTEXT_MINOR_VERSION, opengl_minor,
      WGL_CONTEXT_PROFILE_MASK,  (opengl_compat ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT : WGL_CONTEXT_CORE_PROFILE_BIT), // Ignored for 3.1 and earlier
      WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
      0, 0
    };
    
    shared_rc = wglCreateContextAttribs (shared_dc, 0, attribs);

    wglMakeCurrent (0, 0);

    if (!shared_rc)
      throw Rk::Exception ("Error creating shared render context");

    prepping_frame = 0;
  }
  catch (...)
  {
    cleanup ();
    throw;
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

    logger = 0;
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
    log () << "- Creating render context on GLRenderer\n";
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
    Rk::require (!thread, "Renderer already running");

    ready_frames.clear ();
    free_frames.clear ();

    for (uint i = 0; i != frame_count; i++)
    {
      frames [i].id = i + 1;
      free_frames.push_back (&frames [i]);
    }

    log () << "- GLRenderer starting\n";

    thread.execute (
      [this] { loop (); }
    );
  }

  void GLRenderer::stop ()
  {
    if (!thread)
      return;

    Rk::require (!prepping_frame, "Attempt to stop renderer while preparing frame");

    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (0);
    lock = nil;
    
    log () << "- GLRenderer stopping\n";

    thread.join ();
  }
  
  //
  // Frame specification
  //
  Frame* GLRenderer::begin_frame (float prev_time, float current_time, u32& old_frame_id, u32& new_frame_id)
  {
    Rk::require (!prepping_frame, "Current frame must be submitted before requesting a new one");

    while (!prepping_frame)
    {
      auto lock = free_frames_mutex.get_lock ();
      free_frames.pop_front (prepping_frame);
    }
    
    old_frame_id = prepping_frame -> reset (prev_time, current_time, frame_count, new_frame_id);

    return prepping_frame;
  }

  void GLRenderer::submit_frame (Frame* frame)
  {
    auto gl_frame = static_cast <GLFrame*> (frame);

    Rk::require (prepping_frame == gl_frame, "Submitted frame does not match with tracked current frame");

    prepping_frame -> set_size (width, height);

    auto lock = ready_frames_mutex.get_lock ();
    ready_frames.push_back (gl_frame);

    prepping_frame = 0;
  }

  void GLRenderer::add_garbage_vao (u32 vao)
  {
    Rk::require (prepping_frame != 0, "VAO disposal request with no current frame");
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
