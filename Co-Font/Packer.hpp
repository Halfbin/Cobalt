//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_FONT_H_PACKER
#define CO_FONT_H_PACKER

#include <Co/IxFontFactory.hpp>

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

  class FontPack :
    Rk::NoCopy
  {
  public:
    struct GlyphMetrics
    {
      uint x,
           y,
           width,
           height,
           bearing_x,
           bearing_y,
           advance;

      GlyphMetrics (const Nil& n = nil) :
        x (0),
        y (0),
        width (0),
        height (0),
        bearing_x (0),
        bearing_y (0),
        advance (0)
      { }
      
      GlyphMetrics (uint x, uint y, uint w, uint h, uint bx, uint by, uint a) :
        x         (x),
        y         (y),
        width     (w),
        height    (h),
        bearing_x (bx),
        bearing_y (by),
        advance   (a)
      { }
      
    };
    
  private:
    std::unique_ptr <GlyphMetrics []> metrics;
    std::unordered_map <char32, uint> cp_map;

  public:
    FontPack (const Nil& n = nil)
    { }

    FontPack (
      Rk::Image&       image,
      Rk::StringRef    path,
      uint             index,
      uint             size,
      FontSizeMode     mode,
      const CodeRange* ranges,
      const CodeRange* end,
      float            threshold
    );

    FontPack (FontPack&& other) :
      metrics (std::move (other.metrics)),
      cp_map  (std::move (other.cp_map ))
    { }
    
    FontPack& operator = (FontPack&& other)
    {
      metrics = std::move (other.metrics);
      cp_map  = std::move (other.cp_map );
      return *this;
    }

    uint get_glyph_index (char32_t cp)
    {
      auto iter = cp_map.find (cp);
      if (iter == cp_map.end ())
        throw Rk::Exception () << "Co::FontPack::get_glyph - glyph for U+" << u32 (cp) << " not loaded";
    }

    const GlyphMetrics& get_glyph (uint index)
    {
      return metrics [index];
    }

    template <typename InIter, typename OutIter>
    void get_glyph_indices (InIter cps, InIter cps_end, OutIter indices)
    {
      while (cps != cps_end)
        *indices++ = get_glyph_index (*cps++);
    }

  };

} // namespace Co

#endif
