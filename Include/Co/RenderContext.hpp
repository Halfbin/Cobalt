//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_RENDERCONTEXT
#define CO_H_RENDERCONTEXT

#include <Co/GeomCompilation.hpp>
#include <Co/GeomBuffer.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/TexImage.hpp>

namespace Co
{
  enum TexImageWrap :
    u32
  {
    texwrap_clamp = 0,
    texwrap_wrap  = 1
  };
  
  enum TexImageType :
    u32
  {
    textype_2d        = 0,
    textype_rectangle = 1,
    textype_cube      = 2
  };
  
  enum IndexType :
    u32
  {
    index_none = 0,
    index_u8,
    index_u16,
    index_u32
  };
  
  class RenderContext
  {
  public:
    typedef std::shared_ptr <RenderContext> Ptr;

    virtual GeomBuffer::Ptr create_buffer (
      WorkQueue&  queue, 
      uptr        size = 0,
      const void* data = 0
    ) = 0;

    virtual GeomCompilation::Ptr create_compilation (
      WorkQueue&        queue,
      const GeomAttrib* attribs,
      const GeomAttrib* attribs_end,
      GeomBuffer::Ptr   elements,
      GeomBuffer::Ptr   indices,
      IndexType         index_type
    ) = 0;

    template <typename Container>
    GeomCompilation::Ptr create_compilation (
      WorkQueue&       queue,
      const Container& attribs,
      GeomBuffer::Ptr  elements,
      GeomBuffer::Ptr  indices,
      IndexType        index_type)
    {
      return create_compilation (queue, std::begin (attribs), std::end (attribs), elements, indices, index_type);
    }

    virtual TexImage::Ptr create_tex_image (
      WorkQueue&   queue,
      uint         level_count,
      TexImageWrap wrap,
      bool         filter,
      TexImageType type
    ) = 0;

    virtual void flush () = 0;

  }; // class RenderContext

} // namespace Co

#endif
