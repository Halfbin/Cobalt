//
// Copyright (C) 2011 Roadkill Software
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
  
  struct CodeRange
  {
    char32 begin,
           end;
  };

  class Font
  {
    virtual void retrieve (TexImage::Ptr& tex, GlyphMetrics::Vector& mets, uint& font_height) const = 0;

    mutable TexImage::Ptr        tex;
    mutable GlyphMetrics::Vector mets;
    mutable uint                 font_height;

  public:
    typedef std::shared_ptr <Font> Ptr;
    
    virtual void translate_codepoints (const char32* codepoints, const char32* end, Character* chars) const = 0;
    
    void translate_codepoints (const char32* codepoints, uint count, Character* chars) const
    {
      translate_codepoints (codepoints, codepoints + count, chars);
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

  static void draw_ui_text (
    Frame&           frame,
    Vector2          prev_pos,
    Vector2          cur_pos,
    Font&            font,
    Rk::U16StringRef text,
    Vector4          prev_colour,
    Vector4          cur_colour)
  {
    if (!font.ready ())
      return;

    Rk::ShortU32String <1024> buffer;
    for (auto iter = text.begin (); iter != text.end (); iter++)
      buffer.append (char32 (*iter));

    Co::Character chars [1024];
    font.translate_codepoints (buffer.data (), text.length (), chars);

    Co::TexRect rects [1024];

    int x, y;

    for (uint i = 0; i != text.length (); i++)
    {
      const auto& character = chars [i];
      const auto& metrics   = font.metrics (character.index);
      auto&       rect      = rects [i];

      if (i == 0)
      {
        x = -int (metrics.bearing_x);
        y = metrics.bearing_y;
      }

      x += character.kerning;
      rect.x = x + metrics.bearing_x;
      rect.y = y - metrics.bearing_y;
      rect.w = metrics.width;
      rect.h = metrics.height;
      rect.s = metrics.x;
      rect.t = metrics.y;
      rect.tw = metrics.width;
      rect.th = metrics.height;
      x += metrics.advance;
    }

    frame.add_label (
      font.image (),
      rects,
      text.length (),
      Spatial2D (prev_pos, Co::Complex (1, 0)),
      Spatial2D (cur_pos,  Co::Complex (1, 0)),
      prev_colour,
      cur_colour,
      Vector4 (0, 0, 0, 0),
      Vector4 (0, 0, 0, 0)
    );
  }

  class FontFactory
  {
  public:
    static Rk::StringRef ix_name () { return "Co::FontFactory"; }
    typedef std::shared_ptr <FontFactory> Ptr;

    virtual Font::Ptr create (
      WorkQueue&       queue,
      Rk::StringRef    path,
      uint             size,
      FontSizeMode     mode,
      const CodeRange* ranges,
      const CodeRange* end,
      uint             index = 0
    ) = 0;

    template <typename Container>
    Font::Ptr create (
      WorkQueue&       queue,
      Rk::StringRef    path,
      uint             size,
      FontSizeMode     mode,
      const Container& ranges,
      uint             index = 0)
    {
      return create (queue, path, size, mode, std::begin (ranges), std::end (ranges), index);
    }
    
  };

} // namespace Co

#endif
