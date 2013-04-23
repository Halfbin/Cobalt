//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_FONT
#define CO_H_FONT

// Uses
#include <Co/WorkQueue.hpp>
#include <Co/TexImage.hpp>
#include <Co/Frame.hpp>

#include <Rk/ShortString.hpp>
#include <Rk/StringRef.hpp>
#include <Rk/Types.hpp>

#include <utility>
#include <memory>
#include <vector>

namespace Co
{
  struct GlyphMetrics
  {
    typedef std::vector <GlyphMetrics> Vector;

    u32 x,
        y,
        width,
        height,
        bearing_x,
        bearing_y,
        advance;

    GlyphMetrics (Nil n = nil) :
      x (0),
      y (0),
      width (0),
      height (0),
      bearing_x (0),
      bearing_y (0),
      advance (0)
    { }
    
    GlyphMetrics (uint x, uint y, uint w, uint h, uint bx, uint by, uint adv) :
      x         (x),
      y         (y),
      width     (w),
      height    (h),
      bearing_x (bx),
      bearing_y (by),
      advance   (adv)
    { }
    
  };

  struct Character
  {
    u32 index;
    i32 kerning;

    Character (const Nil& n = nil) :
      index   (0),
      kerning (0)
    { }
    
    Character (u32 index, i32 kerning) :
      index   (index),
      kerning (kerning)
    { }
    
  };

  enum FontSizeMode : u32
  {
    fontsize_points = 0,
    fontsize_pixels = 1
  }; 
  
  // Half open
  struct CodeRange
  {
    char32 begin,
           end;
  };

  struct RangeSet
  {
    const CodeRange* b;
    const CodeRange* e;

    const CodeRange* begin () const { return b; }
    const CodeRange* end   () const { return e; }

    RangeSet (const CodeRange* b, const CodeRange* e) :
      b (b),
      e (e)
    { }
    
    template <typename Ranges>
    RangeSet (const Ranges& ranges) :
      b (std::begin (ranges)),
      e (std::end   (ranges))
    { }
    
    operator bool () const { return b && e; }
    
  };

  static inline const CodeRange* begin (const RangeSet& rs) { return rs.begin (); }
  static inline const CodeRange* end   (const RangeSet& rs) { return rs.end   (); }
  
  //
  // = Font ============================================================================================================
  //
  class Font
  {
    virtual void retrieve (TexImage::Ptr& tex, GlyphMetrics::Vector& mets, uint& font_height) const = 0;

    mutable TexImage::Ptr        tex;
    mutable GlyphMetrics::Vector mets;
    mutable uint                 font_height;

  public:
    typedef std::shared_ptr <Font> Ptr;
    
    virtual void translate_codepoints (const char32* codepoints, const char32* end, Character* chars) const = 0;
    virtual void translate_utf16      (const char16* utf16,      const char16* end, Character* chars) const = 0;

    void translate_codepoints (const char32* codepoints, uptr count, Character* chars) const
    {
      translate_codepoints (codepoints, codepoints + count, chars);
    }

    void translate_utf16 (const char16* utf16, uptr count, Character* chars) const
    {
      translate_utf16 (utf16, utf16 + count, chars);
    }

    GlyphMetrics metrics (uint index) const
    {
      if (ready ())
        return mets [index];
      else
        return GlyphMetrics ();
    }

    TexImage::Ptr image () const
    {
      ready ();
      return tex;
    }
    
    uint height () const
    {
      if (ready ())
        return font_height;
      else
        return 0;
    }

    bool ready () const
    {
      if (!tex)
        retrieve (tex, mets, font_height);
      return tex;
    }

  };

  static inline Co::TexRect make_char_rect (int x, int y, Co::GlyphMetrics mets)
  {
    return Co::TexRect (
      x + mets.bearing_x,
      y - mets.bearing_y,
      mets.width,
      mets.height,
      mets.x,
      mets.y,
      mets.width,
      mets.height
    );
  }
  
  //
  // draw_ui_text
  //
  static void draw_ui_text (
    Frame&           frame,
    v2f              pos,
    Font&            font,
    Rk::U16StringRef text,
    v4f              colour)
  {
    if (!font.ready ())
      return;

    auto len = std::min (text.length (), 1024ull);

    Co::Character chars [1024];
    font.translate_utf16 (text.data (), len, chars);

    Co::TexRect rects [1024];

    int x, y;

    for (uint i = 0; i != len; i++)
    {
      const auto& character = chars [i];
      const auto& metrics   = font.metrics (character.index);
      
      if (i == 0)
      {
        x = -int (metrics.bearing_x);
        y = metrics.bearing_y;
      }

      x += character.kerning;
      rects [i] = make_char_rect (x, y, metrics);
      x += metrics.advance;
    }

    frame.add_label (
      font.image (),
      rects,
      len,
      Spatial2D (pos, Co::Complex (1, 0)),
      colour,
      v4f (0, 0, 0, 0)
    );
  }

  //
  // draw_ui_text_wrap
  //
  static void draw_ui_text_wrap (
    Frame&           frame,
    v2f              pos,
    uint             max_width,
    Font&            font,
    Rk::U16StringRef text,
    v4f              colour)
  {
    if (!font.ready ())
      return;

    auto len = std::min (text.length (), 3000ull);

    Co::Character chars [3000];
    font.translate_utf16 (text.data (), len, chars);

    Co::TexRect rects [3000];

    int x, y;

    for (uint i = 0; i != len; i++)
    {
      const auto& character = chars [i];
      const auto& metrics   = font.metrics (character.index);
      
      if (i == 0)
      {
        x = -int (metrics.bearing_x);
        y = metrics.bearing_y;
      }

      if (x + character.kerning + metrics.bearing_x + metrics.width > int (max_width))
      {
        // New line
        x = -int (metrics.bearing_x);
        y += font.height ();
        y += metrics.bearing_y;
      }

      x += character.kerning;
      rects [i] = make_char_rect (x, y, metrics);
      x += metrics.advance;
    }

    frame.add_label (
      font.image (),
      rects,
      len,
      Spatial2D (pos, Co::Complex (1, 0)),
      colour,
      v4f (0, 0, 0, 0)
    );
  }

  //
  // = FontFactory =====================================================================================================
  //
  class FontFactory
  {
  public:
    struct Params { Log& log; WorkQueue& queue; };
    typedef std::shared_ptr <FontFactory> Ptr;

    virtual Font::Ptr create (
      Rk::StringRef    path,
      uint             size,
      FontSizeMode     mode,
      const CodeRange* ranges,
      const CodeRange* end,
      uint             index = 0
    ) = 0;

    virtual RangeSet get_repetoire (Rk::StringRef name) = 0;

    template <typename Container>
    Font::Ptr create (
      Rk::StringRef    path,
      uint             size,
      FontSizeMode     mode,
      const Container& ranges,
      uint             index = 0)
    {
      return create (path, size, mode, begin (ranges), end (ranges), index);
    }
    
  };
  
  class FontRoot
  {
  public:
    virtual FontFactory::Ptr create_factory (Log& log, WorkQueue& queue) = 0;

  };

} // namespace Co

#endif
