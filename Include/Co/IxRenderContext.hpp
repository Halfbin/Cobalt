//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXRENDERCONTEXT
#define CO_H_IXRENDERCONTEXT

#include <Rk/IxUnique.hpp>

namespace Co
{
  class IxGeomBuffer;
  class IxGeomCompilation;
  class IxTexImage;
  class IxTexRectangle;
  struct GeomAttrib;
  struct GlyphMapping;

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
    textype_rectangle = 1
  };
  
  enum TexImageFilter :
    u32
  {
    texfilter_none      = 0,
    texfilter_linear    = 1,
    texfilter_trilinear = 2
  };
  
  enum IndexType :
    u32
  {
    index_none = 0,
    index_u8,
    index_u16,
    index_u32
  };
  
  class IxRenderContext :
    public Rk::IxUnique
  {
  public:
    typedef Rk::IxUniquePtr <IxRenderContext> Ptr;

    virtual IxGeomBuffer* create_buffer (
      uptr        size = 0,
      const void* data = 0
    ) = 0;

    virtual IxGeomCompilation* create_compilation (
      const GeomAttrib* attributes,
      uint              atrtib_count,
      IxGeomBuffer*     elements,
      IxGeomBuffer*     indices,
      IndexType         index_type
    ) = 0;

    virtual IxTexImage* create_tex_image (
      uint           level_count,
      TexImageWrap   wrap,
      TexImageFilter filter,
      TexImageType   type
    ) = 0;
    
    /*virtual IxTexRectangle* create_tex_rectangle (
      const void* data,
      TexFormat   format,
      uint        width,
      uint        height,
      uptr        size = 0
    ) = 0;*/

    virtual void flush () = 0;

  }; // class IxRenderContext

} // namespace Co

#endif
