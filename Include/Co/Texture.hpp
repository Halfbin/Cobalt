//
// Copyright (C) 2011 Roadkill Software
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

  class TextureFactory
  {
  public:
    static Rk::StringRef ix_name () { return "Co::TextureFactory"; }
    typedef std::shared_ptr <TextureFactory> Ptr;

    virtual Texture::Ptr create (WorkQueue& queue, Rk::StringRef path, bool wrap, bool min_filter, bool mag_filter) = 0;

  };

} // namespace Co

#endif
