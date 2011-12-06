//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXTEXTUREFACTORY
#define CO_H_IXTEXTUREFACTORY

#include <Rk/StringRef.hpp>

namespace Co
{
  class IxLoadContext;
  class IxTexture;

  class IxTextureFactory
  {
  public:
    static const u64 id = 0xa87fa01d6657089aull;

    virtual IxTexture* create (IxLoadContext* context, Rk::StringRef path, bool wrap, bool filter) = 0;

  };

} // namespace Co

#endif
