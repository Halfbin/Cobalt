//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLTEXIMAGE
#define CO_GLRENDERER_H_GLTEXIMAGE

// Implements
#include <Co/TexImage.hpp>

// Uses
#include <Co/RenderContext.hpp>

#include "GL.hpp"

namespace Co
{
  class GLTexImage :
    public TexImage
  {
    u32 name,
        target;

  protected:
    virtual void load_map (u32 sub_image, u32 level, const void* data, TexFormat format, u32 width, u32 height, uptr size = 0);

    GLTexImage (uint level_count, TexImageWrap wrap, bool min_filter, bool mag_filter, TexImageType type);
    
  public:
    typedef std::shared_ptr <GLTexImage> Ptr;

    static inline Ptr create (
      WorkQueue&   queue,
      uint         level_count,
      TexImageWrap wrap,
      bool         min_filter,
      bool         mag_filter,
      TexImageType type)
    {
      return Ptr (
        new GLTexImage (level_count, wrap, min_filter, mag_filter, type),
        queue.make_deleter <GLTexImage> ()
      );
    }

    ~GLTexImage ();
    
    void bind (u32 unit)
    {
      if (!glIsTexture (name))
      {
        unbind (unit);
        return;
      }

      glActiveTexture (GL_TEXTURE0 + unit);
      check_gl ("glActiveTexture");

      glBindTexture (target, name);
      check_gl ("glBindTexture");
    }

    static void unbind (u32 unit)
    {
      glActiveTexture (GL_TEXTURE0 + unit);
      check_gl ("glActiveTexture");
      
      glBindTexture (GL_TEXTURE_2D, 0);
      check_gl ("glBindTexture");
    }

  };

}

#endif
