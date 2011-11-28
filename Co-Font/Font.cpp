//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxFontFactory.hpp>
#include <Co/IxFont.hpp>

// Uses
#include <Co/IxRenderContext.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/IxTexImage.hpp>
#include <Co/Profile.hpp>

#include <Rk/ShortString.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Expose.hpp>
#include <Rk/Image.hpp>
#include <Rk/File.hpp>

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H

namespace Co
{
  namespace
  {
    // Log
    Rk::VirtualLockedOutStream log;
    
    // Freetype
    FT_Library ft = 0;

    //
    // = Glyph =========================================================================================================
    // An individual character glyph, used for packing
    //
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
        best_x   (other.best_x),
        best_y   (other.best_y),
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

        metrics = other.metrics;

        best_x = other.best_x;
        best_y = other.best_y;

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
        using std::swap;
        swap (a.bitmap,   b.bitmap);
        swap (a.metrics,  b.metrics);
        swap (a.best_x,   b.best_x);
        swap (a.best_y,   b.best_y);
        swap (a.index,    b.index);
        swap (a.max_side, b.max_side);
        swap (a.area,     b.area);
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

    //
    // = Rect ==========================================================================================================
    // A rectangle used for packing
    //
    struct Rect
    {
      uint x, y,
           width, height;

      Rect () { }

      __forceinline Rect (uint x, uint y, uint w, uint h) :
        x (x), y (y), width (w), height (h)
      { }
      
      uint area () const
      {
        return width * height;
      }

      bool operator == (const Rect& other) const
      {
        return x == other.x && y == other.y && width == other.width && height == other.height;
      }

      bool operator < (const Rect& other) const
      {
        return area () < other.area ();
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
      //Profiler prof ("Co-Font: pack_glyphs", log);

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
    float make_atlas (Iter first, Iter end, float threshold, uint& best_width, uint& best_height)
    {
      //Profiler prof ("Co-Font: make_atlas", log);

      if (first == end)
        throw Rk::Exception ("X Co-Font: make_atlas - no glyphs to pack");

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

      return float (glyph_area) / float (best_width * best_height);
    } // make_atlas

    //
    // = create_font =====================================================================================================
    //
    template <typename CharMap, typename Iter>
    std::unique_ptr <GlyphMetrics []> create_font (
      FT_Face&      face,
      CharMap&      char_map,
      Rk::Image&    image,
      Rk::StringRef path,
      uint          index,
      uint          size,
      FontSizeMode  mode,
      Iter          ranges,
      Iter          end,
      float         threshold)
    {
      FT_Error error;

      if (!ft)
      {
        error = FT_Init_FreeType (&ft);
        if (error)
          throw Rk::Exception ("X Co-Font: create_font - FT_Init_FreeType failed");
      }

      Rk::ShortString <512> path_buf = path;
      error = FT_New_Face (ft, path_buf.c_str (), 0, &face);
      if (error)
        throw Rk::Exception ("X Co-Font: create_font - FT_New_Face failed");

      error = FT_Set_Char_Size (face, 0, size << 6, 96, 96);
      if (error)
        throw Rk::Exception ("X Co-Font: create_font - FT_Set_Char_Size failed");

      std::map <uint, uint> glyph_remaps; // Remaps FreeType glyph indices to private glyph indices
    
      uint glyph_count = 0;

      for (auto range = ranges; range != end; range++)
      {
        for (char32 cp = range -> begin; cp != range -> end; cp++)
        {
          uint freetype_index = FT_Get_Char_Index (face, cp);
          if (freetype_index == 0)
          {
            // No glyph for this character
            char_map [cp] = ~u32 (0);
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
          throw Rk::Exception ("X Co-Font: create_font - FT_Load_Glyph failed");

        glyphs [remap -> second].init (face -> glyph, remap -> second);
      }
      
      // Sort glyphs by longest side for packing
      std::sort (
        glyphs.begin (),
        glyphs.end   (),
        [] (const Glyph& a, const Glyph& b)
        {
          return a.max_side > b.max_side;
        }
      );

      // Pack
      float efficiency = make_atlas (glyphs.begin (), glyphs.end (), threshold, image.width, image.height);
      log.set_precision (2);
      log << "- Co-Font: " << path << " packed to " << efficiency * 100.0f << "% efficiency\n";

      // Render the packed glyphs to an image
      image.pixel_type   = image.i8;
      image.row_stride   = image.width;
      image.bottom_up    = false;
      image.pixel_stride = 1;
      image.allocate ();
      image.fill (255);

      for (auto iter = glyphs.begin (); iter != glyphs.end (); iter++)
        iter -> blit (image);

      #pragma pack (push)
      #pragma pack (1)

      struct TGAHeader
      {
        u8  id_length,
            palette_type,
            image_type;
        u16 palette_offset,
            palette_entries;
        u8  palette_bpp;
        u16 x_orig, y_orig,
            width, height;
        u8  bpp,
            flags;
      };

      static_assert (sizeof (TGAHeader) == 18, "TGAHeader miscompiled");

      #pragma pack (pop)

      // DEBUG: Dump image 
      TGAHeader head = {
        0,
        1,
        1,
        0, 256, 24,
        0,
        0,
        image.width,
        image.height,
        8,
        0x20
      };

      u8 palette [256 * 3];
      for (uint i = 0; i != 256; i++)
      {
        palette [i * 3 + 0] = u8 (i);
        palette [i * 3 + 1] = u8 (i);
        palette [i * 3 + 2] = u8 (i);
      }

      Rk::File dump ("font-dump.tga", Rk::File::open_replace_or_create);
      dump.put (head)
          .put (palette)
          .write (image.data, image.size ());

      // Re-sort glyphs by private index for use and caching
      std::sort (
        glyphs.begin (),
        glyphs.end   (),
        [] (const Glyph& a, const Glyph& b)
        {
          return a.index < b.index;
        }
      );

      std::unique_ptr <GlyphMetrics []> metrics (new GlyphMetrics [glyph_count]);
      uint dest_index = 0;
      for (auto source = glyphs.begin (); source != glyphs.end (); source++)
      {
        metrics [dest_index] = source -> get_metrics ();
        //metrics [dest_index].y = image.height - metrics [dest_index].y;
        dest_index++;
      }
      
      return std::move (metrics);
    }

    //
    // = Font ============================================================================================================
    //
    class Font :
      public IxFont
    {
      // Parameters
      Rk::ShortString <512>   path;
      uint                    size,
                              index;
      FontSizeMode            mode;
      std::vector <CodeRange> code_ranges;

      // Data
      FT_Face                           face;
      std::unordered_map <char32, u32>  char_map;
      std::unique_ptr <GlyphMetrics []> metrics_ptr;
      IxTexImage::Ptr                   image_ptr;

      // Management
      long ref_count;

      virtual void acquire ();
      virtual void release ();
      virtual void dispose ();

      virtual void load (IxRenderContext& rc);

      virtual void translate_codepoints (const char32* begin, const char32* end, Character* chars);

      ~Font ();

    public:
      template <typename Iter>
      Font (
        IxLoadContext& context,
        Rk::StringRef  new_path,
        uint           new_size,
        FontSizeMode   new_mode,
        Iter           ranges,
        Iter           end,
        uint           new_index
      );

      bool dead () const;

    }; // class Font

    void Font::acquire ()
    {
      _InterlockedIncrement (&ref_count);
    }

    void Font::release ()
    {
      _InterlockedDecrement (&ref_count);
    }

    void Font::dispose ()
    {
      delete this;
    }

    void Font::load (IxRenderContext& rc)
    {
      Rk::Image image;
      metrics_ptr = create_font (face, char_map, image, path, index, size, mode, code_ranges.begin (), code_ranges.end (), 0.95f);
      metrics = metrics_ptr.get ();

      image_ptr = rc.create_tex_image (1, teximage_clamp, teximage_rect);
      image_ptr -> load_map (0, image.data, tex_r8, image.width, image.height, image.size ());
      tex = image_ptr.get ();

      IxResource::ready = true;
    }

    void Font::translate_codepoints (const char32* begin, const char32* end, Character* chars)
    {
      if (!begin || !end || !chars)
        throw Rk::Exception ("X Co-Font: IxFont::translate_codepoints - null pointer");

      if (end < begin)
        throw Rk::Exception ("X Co-Font: IxFont::translate_codepoints - invalid range");

      u32 prev_ft_index = 0;

      while (begin != end)
      {
        auto iter = char_map.find (*begin++);
        if (iter != char_map.end ())
        {
          u32 ft_index = FT_Get_Char_Index (face, iter -> first);

          FT_Vector kerning = { 0, 0 };
          if (prev_ft_index && iter -> first)
            FT_Get_Kerning (face, prev_ft_index, ft_index, 0, &kerning);

          *chars++ = Character (iter -> second, kerning.x >> 6);

          prev_ft_index = ft_index;
        }
        else
        {
          *chars++ = Character (0, 0);
        }
      }
    }

    template <typename Iter>
    Font::Font (
      IxLoadContext& context,
      Rk::StringRef  new_path,
      uint           new_size,
      FontSizeMode   new_mode,
      Iter           ranges,
      Iter           end,
      uint           new_index
    ) :
      size        (new_size),
      index       (new_index),
      mode        (new_mode),
      code_ranges (ranges, end),
      ref_count   (1)
    {
      path  = context.get_game_path ();
      path += "Fonts/";
      path += new_path;

      context.load (this);
    }
    
    Font::~Font ()
    {
      FT_Done_Face (face);
    }

    bool Font::dead () const
    {
      return ref_count == 0;
    }

    //
    // = FontFactory =====================================================================================================
    //
    class FontFactory :
      public IxFontFactory
    {
      typedef std::unordered_map <Rk::ShortString <512>, IxFont*> CacheType;
      CacheType cache;

      virtual void init (Rk::IxLockedOutStreamImpl* log_impl);
      
      virtual IxFont* create (
        IxLoadContext&   context,
        Rk::StringRef    path,
        uint             size,
        FontSizeMode     mode,
        const CodeRange* ranges,
        const CodeRange* end,
        uint             index
      );

    public:
      void expose (void** out, u64 ixid);

    } factory;

    void FontFactory::init (Rk::IxLockedOutStreamImpl* log_impl)
    {
      log.set_impl (log_impl);
    }

    IxFont* FontFactory::create (
      IxLoadContext&   context,
      Rk::StringRef    path,
      uint             size,
      FontSizeMode     mode,
      const CodeRange* ranges,
      const CodeRange* end,
      uint             index)
    {
      if (!log.get_impl ())
        throw Rk::Exception ("X Co-Font: IxFontFactory::create - Factory not initialized");

      IxFont* font;

      Rk::ShortStringOutStream <512> lookup (path);
      lookup << path << '-' << index << '-' << size;
      if (mode == fontsize_points)
        lookup << "pt";
      else
        lookup << "px";

      auto iter = cache.find (lookup);

      if (iter != cache.end ())
      {
        font = iter -> second;
        font -> acquire ();
      }
      else
      {
        font = new Font (context, path, size, mode, ranges, end, index);
        cache.insert (CacheType::value_type (lookup, font));
      }
      
      return font;
    }

    void FontFactory::expose (void** out, u64 ixid)
    {
      Rk::expose <IxFontFactory> (this, ixid, out);
    }

  } // namespace

  IX_EXPOSE (void** out, u64 ixid)
  {
    factory.expose (out, ixid);
  }

} // namespace Co
