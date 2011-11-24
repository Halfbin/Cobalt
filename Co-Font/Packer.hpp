//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_FONT_H_PACKER
#define CO_FONT_H_PACKER

#include <Co/IxFontFactory.hpp>
#include <Co/IxFont.hpp>

#include <Rk/Exception.hpp>
#include <Rk/Image.hpp>

#include <unordered_map>
#include <memory>

namespace Co
{
  class IxTexImage;

  struct CodeRange
  {
    char32 begin,
           end;
  };

  typedef std::unordered_map <char32, u32> CharMap;

  std::unique_ptr <GlyphMetrics []> create_font (
    CharMap&         char_map,
    Rk::Image&       image,
    Rk::StringRef    path,
    uint             index,
    uint             size,
    FontSizeMode     mode,
    const CodeRange* ranges,
    const CodeRange* end,
    float            threshold
  );

} // namespace Co

#endif
