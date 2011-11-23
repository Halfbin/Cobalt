//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXFONTFACTORY
#define CO_H_IXFONTFACTORY

#include <Rk/StringRef.hpp>

namespace Co
{
  class IxLoadContext;
  class IxFont;

  enum FontSizeMode : u32
  {
    fontsize_points = 0,
    fontsize_pixels = 1
  };
  
  class IxFontFactory
  {
  public:
    static const u64 id = 0xee56399853afa81dull;

    virtual IxFont* create (
      IxLoadContext& context,
      Rk::StringRef  path,
      uint           size,
      FontSizeMode   mode,
      uint           index = 0
    ) = 0;

  };

} // namespace Co

#endif
