//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_COMMON
#define CO_GLRENDERER_H_COMMON

#include <Rk/Exception.hpp>
#include <gl/glew.h>

namespace Co
{
namespace
{
  //
  // = Error handling ==================================================================================================
  //
  __declspec(noreturn) void throw_gl (GLenum err, const char* last_call) try
  {
    switch (err)
    {
      case GL_INVALID_ENUM:      throw Rk::Violation ("Invalid enum");
      case GL_INVALID_VALUE:     throw Rk::Exception ("Invalid value");
      case GL_INVALID_OPERATION: throw Rk::Violation ("Invalid operation");
      case GL_OUT_OF_MEMORY:     throw Rk::Exception ("Out of memory");
      default:                   throw Rk::Exception ("Unknown exception");
    }
  }
  catch (...)
  {
    Rk::log_frame (last_call);
    throw;
  }

  __forceinline void check_gl (const char* last_call = "gl<unknown>")
  {
    GLenum err = glGetError ();
    if (err)
      throw_gl (err, last_call);
  }
  
} // namespace
} // namespace Co

#endif
