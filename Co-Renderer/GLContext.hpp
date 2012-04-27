//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLCONTEXT
#define CO_GLRENDERER_H_GLCONTEXT

// Implements
#include <Co/RenderContext.hpp>

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
    public RenderContext
  {
    Rk::Mutex& mutex;
    void*      dc;
    void*      rc;

  public:
    typedef std::unique_ptr <GLContext> Ptr;

    GLContext (Rk::Mutex& device_mutex, WGLCCA wglCCA, void* shared_dc, void* shared_rc, void* target, const int* attribs);

    void present ();

    virtual GeomBuffer::Ptr create_buffer (
      WorkQueue&  queue,
      uptr        size = 0,
      const void* data = 0
    );

    virtual GeomCompilation::Ptr create_compilation (
      WorkQueue&        queue,
      const GeomAttrib* attribs,
      const GeomAttrib* attribs_end,
      GeomBuffer::Ptr   elements,
      GeomBuffer::Ptr   indices,
      IndexType         index_type
    );

    virtual TexImage::Ptr create_tex_image (
      WorkQueue&   queue,
      uint         level_count,
      TexImageWrap wrap,
      bool         filter,
      TexImageType type
    );

    virtual void flush ();

    ~GLContext ();

  }; // class GLContext

} // namespace Co

#endif
