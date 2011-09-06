//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXFONT
#define CO_H_IXFONT

// Implements
#include <Co/IxResource.hpp>

// Uses
#include <Co/IxTexImage.hpp>

namespace Co
{
  class IxFont : 
    public IxResource
  {
  protected:
    IxTexImage* image;

  public:
    typedef Rk::IxSharedPtr <IxFont> Ptr;

    /*IxTexImage* get ()
    {
      if (!ready)
        return 0;

      return image;
    }*/

  }; // class IxFont

} // namespace Co

#endif
