//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLCONTEXT
#define CO_GLRENDERER_H_GLCONTEXT

// Implements
#include <Co/IxRenderContext.hpp>

// Uses
#include <Rk/Mutex.hpp>

namespace Co
{
  //
  // Function pointer type for wglCreateContextAttribs
  //
  typedef void* (__stdcall *WGLCCA) (void*, void*, const int*);

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
    virtual IxTexImage*        create_tex_image   (uint level_count, TexImageWrap wrap, TexImageType type);
    
    virtual void flush ();

    ~GLContext ();
    virtual void destroy ();

  }; // class GLContext

} // namespace Co

#endif
