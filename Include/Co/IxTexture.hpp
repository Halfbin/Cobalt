//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXTEXTURE
#define CO_H_IXTEXTURE

// Implements
#include <Co/IxResource.hpp>

// Uses
#include <Co/IxTexImage.hpp>

namespace Co
{
  class IxTexture : 
    public IxResource
  {
  protected:
    IxTexImage* image;

  public:
    typedef Rk::IxSharedPtr <IxTexture> Ptr;

    IxTexImage* get ()
    {
      if (!ready)
        return 0;

      return image;
    }

  }; // class IxModel

} // namespace Co

#endif
