//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLSHADER
#define CO_GLRENDERER_H_GLSHADER

#include <Rk/StringRef.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class GLShader
  {
    u32 name;

  public:
    GLShader (Rk::StringRef path, u32 type);
    ~GLShader ();
    
    u32 get_name () const
    {
      return name;
    }

  };

  class GLProgram
  {
    u32 name;

  public:
    GLProgram (const Nil&);
    GLProgram ();
    ~GLProgram ();

    void add_shader (const GLShader& shader);

    void        use ();
    static void done ();

    void fix_attrib   (const char* name, u32 location);
    void link         ();
    u32  link_uniform (const char* name);

  };

} // namespace Co

#endif
