//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLShader.hpp"

// Uses
#include <Rk/File.hpp>

#include "GL.hpp"

namespace Co
{
  GLShader::GLShader (Rk::StringRef path, u32 type)
  {
    Rk::File file (path, Rk::File::open_read_existing);

    u32 size = u32 (file.size ());
    if (size == 0)
      throw Rk::Exception () << "Co-GLRenderer: GLShader::GLShader - " << path << " is empty";

    std::unique_ptr <char []> buffer (new char [size]);
    file.read (buffer.get (), size);
    
    name = glCreateShader (type);
    check_gl ("glCreateShader");

    const char* data = buffer.get ();
    u32 len = size;
    glShaderSource (name, 1, &data, (const i32*) &len);
    check_gl ("glShaderSource");

    glCompileShader (name);
    check_gl ("glCompileShader");

    glGetShaderiv (name, GL_INFO_LOG_LENGTH, (i32*) &len);
    check_gl ("glGetShaderiv (GL_INFO_LOG_LENGTH)");

    if (len > 1)
    {
      if (len > size)
        buffer.reset (new char [len]);
      glGetShaderInfoLog (name, len, (i32*) &len, buffer.get ());
      check_gl ("glGetShaderInfoLog");
      
      /*log () << "- Infolog from shader compiler:\n"
             << Rk::StringRef (buffer.get (), len - 1)
             << '\n';*/
    }

    int ok;
    glGetShaderiv (name, GL_COMPILE_STATUS, &ok);
    check_gl ("glGetShaderiv (GL_COMPILE_STATUS)");

    if (!ok)
      throw Rk::Exception ("Error compiling shader");
  }

  GLShader::~GLShader ()
  {
    glDeleteShader (name);
    name = 0;
  }

  GLProgram::GLProgram (const Nil&) :
    name (0)
  { }
  
  GLProgram::GLProgram ()
  {
    name = glCreateProgram ();
    check_gl ("glCreateProgram");
  }

  GLProgram::~GLProgram ()
  {
    glDeleteProgram (name);
    name = 0;
  }

  void GLProgram::add_shader (const GLShader& shader)
  {
    glAttachShader (name, shader.get_name ());
    check_gl ("glAttachShader");
  }

  void GLProgram::fix_attrib (const char* attrib, u32 location)
  {
    glBindAttribLocation (name, location, attrib);
    check_gl ("glBindAttribLocation");
  }

  void GLProgram::link ()
  {
    glLinkProgram (name);
    check_gl ("glLinkProgram");

    u32 len;
    glGetProgramiv (name, GL_INFO_LOG_LENGTH, (i32*) &len);
    check_gl ("glGetProgramiv (GL_INFO_LOG_LENGTH)");

    if (len > 1)
    {
      std::unique_ptr <char []> buffer (new char [len]);
      glGetProgramInfoLog (name, len, (i32*) &len, buffer.get ());
      check_gl ("glGetProgramInfoLog");

      /*log () << "- Infolog from program linker:\n"
             << Rk::StringRef (buffer.get (), len - 1)
             << '\n';*/
    }

    int ok;
    glGetProgramiv (name, GL_LINK_STATUS, &ok);
    check_gl ("glGetProgramiv (GL_LINK_STATUS)");

    if (!ok)
      throw Rk::Exception ("Co-GLRenderer: GLProgram::link - Error linking shader program");
  }

  u32 GLProgram::link_uniform (const char* uniform)
  {
    u32 location = glGetUniformLocation (name, uniform);
    check_gl ("glGetUniformLocation");
    if (location == 0xffffffff)
      throw Rk::Exception ("Co-GLRenderer: GLProgram::link_uniform - Uniform not present or inactive");
    return location;
  }

  void GLProgram::use ()
  {
    glUseProgram (name);
    check_gl ("glUseProgram");
  }

  void GLProgram::done ()
  {
    glUseProgram (0);
    check_gl ("glUseProgram");
  }

}
