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
  //class IxGlyphPack;
  struct GeomAttrib;
  struct GlyphMapping;

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
      IxGeomBuffer*     indices
    ) = 0;

    virtual IxTexImage* create_tex_image (
      uint level_count,
      bool wrap
    ) = 0;
    
    /*virtual IxGlyphPack* create_glyph_pack (
      const u8*           atlas,
      uint                width,
      uint                height,
      const GlyphMapping* mappings,
      uint                glyph_count
    ) = 0;*/

    virtual void flush () = 0;

  }; // class IxRenderContext

} // namespace Co

#endif
