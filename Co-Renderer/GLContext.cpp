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
  GLContext::GLContext (Rk::Mutex& device_mutex, WGLCCA wglCCA, void* shared_dc, void* shared_rc, void* target) try :
    mutex (device_mutex)
  {
    auto lock = mutex.get_lock ();
    
    int attribs [] = {
      WGL_CONTEXT_MAJOR_VERSION, opengl_major,
      WGL_CONTEXT_MINOR_VERSION, opengl_minor,
      WGL_CONTEXT_PROFILE_MASK,  (opengl_compat ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT : WGL_CONTEXT_CORE_PROFILE_BIT), // Ignored for 3.1 and earlier
      WGL_CONTEXT_FLAGS,         WGL_CONTEXT_DEBUG_BIT,
      0,                         0
    };
    
    rc = wglCCA (shared_dc, shared_rc, attribs);
    if (!rc)
      throw Rk::Exception ("Error creating render context");
    
    dc = GetDC (target);
    if (!dc)
      throw Rk::Exception ("Error retrieving device context");
    
    bool ok = wglMakeCurrent (dc, rc) != 0;
    if (!ok)
      throw Rk::Exception ("Error making render context current");
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
    glFinish ();
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
  IxTexImage* GLContext::create_tex_image (uint level_count, bool wrap)
  {
    return new GLTexImage (level_count, wrap);
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

}
