//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXRENDERDEVICE
#define CO_H_IXRENDERDEVICE

#include <Rk/Types.hpp>
#include <Rk/IxUnique.hpp>
#include <Co/IxGeomCompilation.hpp>

namespace Co
{
  class IxGeomBuffer;
  class IxTexture;

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

    virtual IxTexture* create_texture (
      uint level_count,
      bool wrap
    ) = 0;
    
  };

  class IxRenderDevice
  {
  public:
    enum : u64 { id = 0x821915e4b144888dull };

    virtual IxRenderContext* create_context () = 0;

  };

} // namespace Co

#endif
