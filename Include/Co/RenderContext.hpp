//
// Copyright (C) 2012 Roadkill Software
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
  class RenderContext
  {
  public:
    typedef std::shared_ptr <RenderContext> Ptr;

    virtual GeomBuffer::Ptr create_buffer (
      WorkQueue&  queue, 
      uptr        size = 0,
      const void* data = 0
    ) = 0;

    virtual StreamBuffer::Ptr create_stream (
      WorkQueue& queue,
      uptr       size
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

    virtual TexImage::Ptr create_tex_image_2d (
      WorkQueue&   queue,
      TexFormat    format,
      u32          width,
      u32          height,
      u32          level_count,
      TexImageWrap wrap,
      bool         min_filter,
      bool         mag_filter,
      const void*  data = nullptr,
      uptr         size = 0
    ) = 0;

    virtual TexImage::Ptr create_tex_rectangle (
      WorkQueue&   queue,
      TexFormat    format,
      u32          width,
      u32          height,
      TexImageWrap wrap,
      const void*  data = nullptr,
      uptr         size = 0
    ) = 0;

    virtual TexImage::Ptr create_tex_cube (
      WorkQueue& queue,
      TexFormat  format,
      u32        width,
      u32        height,
      bool       min_filter,
      bool       mag_filter
    ) = 0;

    virtual TexImage::Ptr create_tex_array (
      WorkQueue&   queue,
      TexFormat    format,
      u32          width,
      u32          height,
      u32          layer_count,
      u32          level_count,
      TexImageWrap wrap,
      bool         min_filter,
      bool         mag_filter
    ) = 0;

    virtual void flush () = 0;

  }; // class RenderContext

} // namespace Co

#endif
