//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "Packer.hpp"

// Uses
#include <Rk/ShortStringOutStream.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Console.hpp>
#include <Rk/MinMax.hpp>
#include <Rk/Image.hpp>
#include <Rk/File.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H

#include <algorithm>
#include <vector>
#include <map>
#include <set>

namespace Co
{
  namespace
  {
    FT_Library ft = 0;

    // Glyph
    // An individual character glyph
    class Glyph
    {
      FT_Bitmap        bitmap;
      FT_Glyph_Metrics metrics;
      uint             best_x,
                       best_y;

    public:
      uint index,
           x, y,
           max_side,
           area;

      Glyph (Glyph&& other) :
        bitmap   (other.bitmap),
        metrics  (other.metrics),
        index    (other.index),
        max_side (other.max_side),
        area     (other.area)
      {
        other.bitmap.buffer = 0;
        other.index         = ~uint (0);
      }
      
      Glyph& operator = (Glyph&& other)
      {
        if (bitmap.buffer)
          FT_Bitmap_Done (ft, &bitmap);

        bitmap = other.bitmap;
        other.bitmap.buffer = 0;

        metrics  = other.metrics;

        index       = other.index;
        other.index = ~uint (0);

        max_side = other.max_side;
        area     = other.area;

        return *this;
      }

      Glyph ()
      {
        bitmap.buffer = 0;
        index         = ~uint (0);
      }

      void init (FT_GlyphSlot slot, uint new_index)
      {
        index = new_index;

        Rk::zero_out (bitmap);

        auto error = FT_Bitmap_Convert (ft, &slot -> bitmap, &bitmap, 1);
        if (error)
          throw Rk::Exception () << "Error copying glyph bitmap";// for glyph #" << index;

        metrics = slot -> metrics;

        max_side = Rk::maximum (width (), height ());
        area = width () * height ();
      }
      
      ~Glyph ()
      {
        if (bitmap.buffer)
          FT_Bitmap_Done (ft, &bitmap);
      }
      
      uint __forceinline height () const
      {
        return bitmap.rows;
      }

      uint __forceinline width () const
      {
        return bitmap.width;
      }

      const void* data () const
      {
        return bitmap.buffer;
      }

      uint size () const
      {
        return std::abs (bitmap.pitch) * bitmap.rows;
      }

      void blit (Rk::Image& atlas) const
      {
        assert (best_x + width  () <= atlas.width );
        assert (best_y + height () <= atlas.height);
    
        u8*       dest   = atlas.data + best_y * atlas.row_stride + best_x;
        const u8* source = bitmap.buffer;

        for (uint j = 0; j != height (); j++)
        {
          for (uint i = 0; i != width (); i++)
          {
            u8 s = *source++;
            dest [i] = s;//(s == 0) ? 1 : s;
          }

          dest += atlas.row_stride;
        }
      }

      void assume_best ()
      {
        best_x = x;
        best_y = y;
      }

      friend void swap (Glyph& a, Glyph& b)
      {
        std::swap (a.bitmap,   b.bitmap);
        std::swap (a.metrics,  b.metrics);
        std::swap (a.index,    b.index);
        std::swap (a.max_side, b.max_side);
        std::swap (a.area,     b.area);
      }

      GlyphMetrics get_metrics () const
      {
        return GlyphMetrics (
          best_x,
          best_y,
          width (),
          height (),
          metrics.horiBearingX >> 6,
          metrics.horiBearingY >> 6,
          metrics.horiAdvance  >> 6
        );
      }

    }; // Glyph

    // Rect
    // A rectangle used for packing
    struct Rect
    {
      uint x, y,
           width, height;

      Rect () { }

      __forceinline Rect (uint x, uint y, uint w, uint h) :
        x (x), y (y), width (w), height (h)
      { }
      
      uint area () const { return width * height; }

      bool operator == (const Rect& other) const
      {
        return x == other.x && y == other.y && width == other.width && height == other.height;
      }

      bool operator < (const Rect& other) const
      {
        return (x < other.x) ? (y < other.y) : false;
      }

    }; // Rect

    //
    // = pack_glyphs =======================================================================================================
    // Attempts to pack the glyphs [first, end) into a width * height rectangle.
    // If the glyphs were packed successfully, then the x and y members of the glyphs
    // are set appropriately, and true is returned.
    // Otherwise, the x and y members of the glyphs are undefined, and false is returned.
    //
    template <typename Iter>
    bool pack_glyphs (Iter first, Iter end, uint width, uint height)
    {
      typedef std::multiset <Rect> Rects;
      Rects rects;
      rects.insert (Rect (0, 0, width, height));

      for (Iter g = first; g != end; g++)
      {
        if (g -> area == 0)
        {
          g -> x = 0;
          g -> y = 0;
          continue;
        }

        bool best_is_tight = false;
        uint best_remainder = ~uint (0);
        Rects::iterator best_rect;

        for (auto rect_iter = rects.begin (); rect_iter != rects.end (); rect_iter++)
        {
          const Rect rect = *rect_iter;

          if (g -> width () > rect.width || g -> height () > rect.height)
            continue; // glyph doesn't fit in this rect

          uint remainder = rect.area () - g -> area;

          if (!best_is_tight || remainder < best_remainder)
          {
            if (g -> width () == rect.width || g -> height () == rect.height)
            {
              // Tight fit - this is preferable
              best_is_tight = true;
              best_remainder = remainder;
              best_rect = rect_iter;
              continue;
            }
          }

          if (!best_is_tight && remainder < best_remainder)
          {
            best_remainder = remainder;
            best_rect = rect_iter;
          }
        }

        if (best_remainder == ~uint (0))
          return false; // Atlas is too small!

        g -> x = best_rect -> x;
        g -> y = best_rect -> y;

        Rect rect = *best_rect;

        if (best_is_tight && best_remainder != 0)
        {
          if (rect.width > g -> width ())
          {
            rect.x     += g -> width ();
            rect.width -= g -> width ();
          }
          else if (rect.height > g -> height ())
          {
            rect.y      += g -> height ();
            rect.height -= g -> height ();
          }

          rects.insert (best_rect, rect);
        }
        else
        {
          uint right_gap  = rect.width  - g -> width  ();
          uint bottom_gap = rect.height - g -> height ();

          if (right_gap > bottom_gap)
          {
            rects.insert (best_rect, Rect (rect.x,                 rect.y + g -> height (), g -> width (), bottom_gap ));
            rects.insert (best_rect, Rect (rect.x + g -> width (), rect.y,                  right_gap,     rect.height));
          }
          else
          {
            rects.insert (best_rect, Rect (rect.x,                 rect.y + g -> height (), rect.width, bottom_gap    ));
            rects.insert (best_rect, Rect (rect.x + g -> width (), rect.y,                  right_gap,  g -> height ()));
          }
        }

        rects.erase (best_rect);
      }

      return true; // packed ok
    } // pack_glyphs

    //
    // = make_atlas ========================================================================================================
    // Tries to find a good packing solution for the glyphs [first, end).
    //
    template <typename Iter>
    void make_atlas (Iter first, Iter end, float threshold, uint& best_width, uint& best_height)
    {
      if (first == end)
        throw Rk::Exception ("Co-Font: make_atlas - no glyphs to pack");

      Rk::clamp (threshold, 0.0f, 1.0f);

      uint glyph_area = 0;
      for (auto g = first; g != end; g++)
        glyph_area += g -> area;

      uint min_side = ((uint) std::sqrt (float (glyph_area)) / 4) & ~15;
      
      best_width  = 65535;
      best_height = 65535;

      for (uint height = min_side;; height += 16)
      {
        if (height * min_side > best_width * best_height)
          break;
        
        for (uint width = min_side;; width += 16)
        {
          uint area = width * height;
          
          if (threshold * area < float (glyph_area))
            continue;
          
          if (area > best_width * best_height)
            break;
          
          if (pack_glyphs (first, end, width, height))
          {
            best_width  = width;
            best_height = height;
            for (auto g = first; g != end; g++)
              g -> assume_best ();
          }
        }
      }
      
      assert (best_width != ~uint (0));
    } // make_atlas

  } // namespace

  //
  // = create_font =====================================================================================================
  //
  std::unique_ptr <GlyphMetrics []> create_font (
    CharMap&         char_map,
    Rk::Image&       image,
    Rk::StringRef    path,
    uint             index,
    uint             size,
    FontSizeMode     mode,
    const CodeRange* ranges,
    const CodeRange* end,
    float            threshold)
  {
    FT_Error error;

    if (!ft)
    {
      error = FT_Init_FreeType (&ft);
      if (error)
        throw Rk::Exception ("Co-Font: make_font - FT_Init_FreeType failed");
    }

    FT_Face face;
    Rk::ShortString <512> path_buf = path;
    error = FT_New_Face (ft, path_buf.c_str (), 0, &face);
    if (error)
      throw Rk::Exception ("Co-Font: make_font - FT_New_Face failed");

    error = FT_Set_Char_Size (face, 0, size << 6, 96, 96);
    if (error)
      throw Rk::Exception ("Co-Font: make_font - FT_Set_Char_Size failed");

    std::map <uint, uint> glyph_remaps; // Remaps FreeType glyph indices to private glyph indices
    
    uint glyph_count = 0;

    for (auto range = ranges; range != end; range++)
    {
      for (uint cp = range -> begin; cp != range -> end; cp++)
      {
        uint freetype_index = FT_Get_Char_Index (face, cp);
        if (freetype_index == 0)
        {
          // No glyph for this character
          char_map [cp] = ~uint (0);
          continue;
        }
        else
        {
          // Character uses a glyph - do we already have it?
          auto remap = glyph_remaps.find (freetype_index);
          if (remap != glyph_remaps.end ())
          {
            // We already have the glyph remapped
            char_map [cp] = remap -> second;
          }
          else
          {
            // We need to remap the glyph
            glyph_remaps [freetype_index] = glyph_count;
            char_map [cp] = glyph_count;
            glyph_count++;
          }
        }
      }
    }

    std::vector <Glyph> glyphs;
    glyphs.resize (glyph_count);

    for (auto remap = glyph_remaps.begin (); remap != glyph_remaps.end (); remap++)
    {
      error = FT_Load_Glyph (face, remap -> first, FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT);
      if (error)
        throw Rk::Exception ("Co-Font: make_font - FT_Load_Glyph failed");

      glyphs [remap -> second].init (face -> glyph, remap -> second);
    }
    
    // Sort glyphs by longest side for packing
    std::sort (
      glyphs.begin (),
      glyphs.end   (),
      [] (const Glyph& a, const Glyph& b) -> bool
      {
        return a.max_side > b.max_side;
      }
    );

    // Pack
    make_atlas (glyphs.begin (), glyphs.end (), threshold, image.width, image.height);

    // Render the packed glyphs to an image
    image.pixel_type   = image.i8;
    image.row_stride   = image.width;
    image.bottom_up    = false;
    image.pixel_stride = 1;
    image.allocate ();
    image.fill (255);

    for (auto iter = glyphs.begin (); iter != glyphs.end (); iter++)
      iter -> blit (image);

    // Re-sort glyphs by private index for use and caching
    std::sort (
      glyphs.begin (),
      glyphs.end   (),
      [] (const Glyph& a, const Glyph& b) -> bool
      {
        return a.index < b.index;
      }
    );

    std::unique_ptr <GlyphMetrics []> metrics (new GlyphMetrics [glyph_count]);
    uint dest_index = 0;
    for (auto source = glyphs.begin (); source != glyphs.end (); source++)
      metrics [dest_index++] = source -> get_metrics ();
      
    FT_Done_Face (face);

    return std::move (metrics);
  }

} // namespace Co
