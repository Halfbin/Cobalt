//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXFONT
#define CO_H_IXFONT

// Implements
#include <Co/IxResource.hpp>

// Uses
//#include <Co/IxGlyphPack.hpp>

namespace Co
{
  class IxTexImage;

  class IxFont : 
    public IxResource
  {
  protected:
    IxTexImage* tex;

  public:
    typedef Rk::IxSharedPtr <IxFont> Ptr;

    
  }; // class IxFont

} // namespace Co

#endif
