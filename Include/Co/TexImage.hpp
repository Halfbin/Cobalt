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
  enum TexImageWrap : u32
  {
    texwrap_clamp = 0,
    texwrap_wrap  = 1
  };
  
  class TexImage
  {
  public:
    typedef std::shared_ptr <TexImage> Ptr;

    virtual uptr load_map (u32 sub_image, u32 level, const void* data, uptr size = 0) = 0;

  };

} // namespace Co

#endif
