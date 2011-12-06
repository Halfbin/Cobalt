//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXTEXIMAGE
#define CO_H_IXTEXIMAGE

#include <Co/TextureFile.hpp>

#include <Rk/IxUnique.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class IxTexImage :
    public Rk::IxUnique
  {
  public:
    typedef Rk::IxUniquePtr <IxTexImage> Ptr;
    virtual bool load_map (u32 sub_image, u32 level, const void* data, TexFormat format, u32 width, u32 height, uptr size = 0) = 0;

  };

} // namespace Co

#endif
