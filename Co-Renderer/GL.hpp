//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GL
#define CO_GLRENDERER_H_GL

#include <Rk/Exception.hpp>

#include <gl/glew.h>
//#include <gl/wglew.h>

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glew32")
#pragma comment (lib, "glu32")

//
// = WGL API ===========================================================================================================
//
enum WGLCCAConstants
{
  WGL_CONTEXT_MAJOR_VERSION    = 0x2091,
  WGL_CONTEXT_MINOR_VERSION    = 0x2092,
  WGL_CONTEXT_PROFILE_MASK     = 0x9126,
  WGL_CONTEXT_FLAGS            = 0x2094,
  WGL_CONTEXT_DEBUG_BIT        = 0x0001,
  WGL_CONTEXT_CORE_PROFILE_BIT          = 0x00000001,
  WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT = 0x00000002
};

extern "C" __declspec(dllimport) u8 wglewIsSupported (const char*);

namespace Co
{
  //
  // = OpenGL version ==================================================================================================
  //
  static const uint opengl_major  = 3,
                    opengl_minor  = 3;
#define opengl_compat 0

  //
  // Error handling
  //
  __declspec(noreturn) static void throw_gl (GLenum err, const char* last_call)
  {
    switch (err)
    {
      case GL_INVALID_ENUM:      throw Rk::Exception ("Invalid enum");
      case GL_INVALID_VALUE:     throw Rk::Exception ("Invalid value");
      case GL_INVALID_OPERATION: throw Rk::Exception ("Invalid operation");
      case GL_OUT_OF_MEMORY:     throw Rk::Exception ("Out of memory");
      default:                   throw Rk::Exception ("Unknown exception");
    }
  }

  static __forceinline void check_gl (const char* last_call = "gl<unknown>")
  {
    GLenum err = glGetError ();
    if (err)
      throw_gl (err, last_call);
  }

} // namespace Co

#endif
