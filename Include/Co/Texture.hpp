//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_TEXTURE
#define CO_H_TEXTURE

// Uses
#include <Co/WorkQueue.hpp>
#include <Co/TexImage.hpp>

#include <Rk/StringRef.hpp>

#include <memory>

namespace Co
{
  class Texture
  {
    TexImage::Ptr image;
    
    virtual TexImage::Ptr retrieve () = 0;

  public:
    typedef std::shared_ptr <Texture> Ptr;

    bool ready ()
    {
      return get () != nullptr;
    }

    TexImage::Ptr get ()
    {
      if (!image)
        image = retrieve ();
      return image;
    }

  }; // class Texture

  static const u8
    texture_wrap      = 0x1,
    texture_minfilter = 0x2,
    texture_magfilter = 0x4,
    texture_filter    = 0x6
  ;

  class TextureFactory
  {
  public:
    typedef std::shared_ptr <TextureFactory> Ptr;

    virtual Texture::Ptr create (
      Rk::StringRef path,
      u8            flags = 0
    ) = 0;

  };
  
  class TextureRoot
  {
  public:
    virtual TextureFactory::Ptr create_factory (Log& log, WorkQueue& queue) = 0;

  };

} // namespace Co

#endif
