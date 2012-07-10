//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXTEXIMAGE
#define CO_H_IXTEXIMAGE

#include <Co/TextureFile.hpp>

#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  class TexImage
  {
  public:
    typedef std::shared_ptr <TexImage> Ptr;

    virtual void load_map (u32 sub_image, u32 level, const void* data, TexFormat format, u32 width, u32 height, uptr size = 0) = 0;

  };

} // namespace Co

#endif
