//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLContext.hpp"

// Uses
#include <Rk/Exception.hpp>

#include "GLCompilation.hpp"
#include "GLTexImage.hpp"
#include "GLBuffer.hpp"
#include "GL.hpp"

extern "C"
{
  __declspec(dllimport) i32   __stdcall wglMakeCurrent      (void*, void*);
  __declspec(dllimport) void  __stdcall wglDeleteContext    (void*);
  __declspec(dllimport) void* __stdcall GetDC               (void*);
  __declspec(dllimport) void  __stdcall ReleaseDC           (void*, void*);
  __declspec(dllimport) void  __stdcall SwapBuffers         (void*);
}

namespace Co
{
  //
  // Constructor
  //
  GLContext::GLContext (Rk::Mutex& device_mutex, WGLCCA wglCCA, void* shared_dc, void* shared_rc, void* target, const int* attribs) try :
    mutex (device_mutex)
  {
    auto lock = mutex.get_lock ();
    
    rc = wglCCA (shared_dc, shared_rc, attribs);
    if (!rc)
      throw std::runtime_error ("Error creating render context");
    
    dc = GetDC (target);
    if (!dc)
      throw std::runtime_error ("Error retrieving device context");
    
    bool ok = wglMakeCurrent (dc, rc) != 0;
    if (!ok)
      throw std::runtime_error ("Error making render context current");
  }
  catch (...)
  {
    if (dc) ReleaseDC (target, dc);
    if (rc) wglDeleteContext (rc);
  }

  //
  // Present
  //
  void GLContext::present ()
  {
    //glFinish ();
    SwapBuffers (dc);
  }

  //
  // Buffer creation
  //
  GeomBuffer::Ptr GLContext::create_buffer (WorkQueue& queue, uptr size, const void* data)
  {
    return GLBuffer::create (queue, false, size, data);
  }

  StreamBuffer::Ptr GLContext::create_stream (WorkQueue& queue, uptr size)
  {
    return GLBuffer::create (queue, true, size);
  }

  //
  // Compilation creation
  //
  GeomCompilation::Ptr GLContext::create_compilation (
    WorkQueue&        queue,
    const GeomAttrib* attribs,
    const GeomAttrib* attribs_end,
    GeomBuffer::Ptr   elements,
    GeomBuffer::Ptr   indices,
    IndexType         index_type)
  {
    return GLCompilation::create (queue, attribs, attribs_end, elements, indices, index_type);
  }

  //
  // Texture creation
  //
  TexImage::Ptr GLContext::create_tex_image_2d (
    WorkQueue&   queue,
    TexFormat    format,
    u32          width,
    u32          height,
    u32          level_count,
    TexImageWrap wrap,
    bool         min_filter,
    bool         mag_filter,
    const void*  data,
    uptr         size)
  {
    return queue.gc_attach (new GLTexImage (format, width, height, level_count, wrap, min_filter, mag_filter, data, size));
  }

  TexImage::Ptr GLContext::create_tex_rectangle (
    WorkQueue&   queue,
    TexFormat    format,
    u32          width,
    u32          height,
    TexImageWrap wrap,
    const void*  data,
    uptr         size)
  {
    return queue.gc_attach (new GLTexImage (format, width, height, wrap, data, size));
  }
  
  TexImage::Ptr GLContext::create_tex_cube (
    WorkQueue& queue,
    TexFormat  format,
    u32        width,
    u32        height,
    bool       min_filter,
    bool       mag_filter)
  {
    return queue.gc_attach (new GLTexImage (format, width, height, min_filter, mag_filter));
  }
  
  TexImage::Ptr GLContext::create_tex_array (
    WorkQueue&   queue,
    TexFormat    format,
    u32          width,
    u32          height,
    u32          layer_count,
    u32          level_count,
    TexImageWrap wrap,
    bool         min_filter,
    bool         mag_filter)
  {
    return queue.gc_attach (new GLTexImage (format, width, height, layer_count, level_count, wrap, min_filter, mag_filter));
  }
  
  //
  // Flush
  //
  void GLContext::flush ()
  {
    glFlush ();
  }

  //
  // Destructors
  //
  GLContext::~GLContext ()
  {
    auto lock = mutex.get_lock ();

    if (dc && rc)
    {
      wglMakeCurrent (0, 0);
      wglDeleteContext (rc);
    }
  }

}
