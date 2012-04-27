//
// Copyright (C) 2011 Roadkill Software
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
    
    /*int attribs [] = {
      WGL_CONTEXT_MAJOR_VERSION, opengl_major,
      WGL_CONTEXT_MINOR_VERSION, opengl_minor,
      WGL_CONTEXT_PROFILE_MASK,  WGL_CONTEXT_CORE_PROFILE_BIT, // Ignored for 3.1 and earlier
    #ifndef NDEBUG
      WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
    #endif
      0,                         0
    };*/
    
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
    return GLBuffer::create (queue, size, data);
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
  TexImage::Ptr GLContext::create_tex_image (
    WorkQueue&   queue,
    uint         level_count,
    TexImageWrap wrap,
    bool         filter,
    TexImageType type)
  {
    return GLTexImage::create (queue, level_count, wrap, filter, type);
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
