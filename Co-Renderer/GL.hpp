//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GL
#define CO_GLRENDERER_H_GL

#include <Rk/Exception.hpp>
#include <Rk/StringRef.hpp>

#include <gl/glew.h>
//#include <gl/wglew.h>

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

  //
  // Error handling
  //
  #ifndef NDEBUG

  class GLError :
    public std::runtime_error
  {
  public:
    GLError (Rk::StringRef message) :
      runtime_error (message.string ())
    { }
    
  };

  __declspec(noreturn) static void throw_gl (GLenum err, const char* last_call)
  {
    switch (err)
    {
      case GL_INVALID_ENUM:      throw GLError ("Invalid enum");
      case GL_INVALID_VALUE:     throw GLError ("Invalid value");
      case GL_INVALID_OPERATION: throw GLError ("Invalid operation");
      case GL_OUT_OF_MEMORY:     throw GLError ("Out of memory");
      default:                   throw GLError ("Unknown exception");
    }
  }

  #endif

  static __forceinline void check_gl (const char* last_call = "gl<unknown>")
  {
  #ifndef NDEBUG
    GLenum err = glGetError ();
    if (err)
      throw_gl (err, last_call);
  #endif
  }

} // namespace Co

#endif
