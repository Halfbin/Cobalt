//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Font.hpp>

// Uses
#include <Co/ResourceFactory.hpp>
#include <Co/RenderContext.hpp>
#include <Co/Filesystem.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/TexImage.hpp>
//#include <Co/Profile.hpp>

#include <Rk/StringOutStream.hpp>
#include <Rk/AsyncMethod.hpp>
#include <Rk/ShortString.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Modular.hpp>
#include <Rk/Guard.hpp>
#include <Rk/Image.hpp>
#include <Rk/File.hpp>

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
  //
  // = FontImpl ======================================================================================================
  //
  class FontImpl :
    public Font
  {
    // Parameters
    std::string             path;
    uint                    size,
                            index;
    FontSizeMode            mode;
    std::vector <CodeRange> code_ranges;

    // Data
    std::unordered_map <char32, u32> char_map;
    GlyphMetrics::Vector             mets;
    TexImage::Ptr                    tex;
    uint                             font_height;
    mutable Rk::Mutex                mutex;
    std::unordered_map <u64, v2i>    kerning_pairs;

    // Management
    virtual void retrieve (TexImage::Ptr& tex, GlyphMetrics::Vector& mets, uint& font_height) const;

    Character translate_codepoint (char32 cp, u32& prev_index) const;

    virtual void translate_codepoints (const char32* begin, const char32* end, Character* chars) const;
    virtual void translate_utf16      (const char16* utf16, const char16* end, Character* chars) const;

    template <typename Iter>
    FontImpl (
      Rk::StringRef  new_path,
      uint           new_size,
      FontSizeMode   new_mode,
      Iter           ranges,
      Iter           end,
      uint           new_index
    );

  public:
    void construct (std::shared_ptr <FontImpl>& self, WorkQueue& queue, LoadContext& ctx);

    template <typename Iter>
    static Ptr create (
      WorkQueue&     queue,
      Rk::StringRef  path,
      uint           size,
      FontSizeMode   mode,
      Iter           ranges,
      Iter           end,
      uint           index
    );

  }; // class FontImpl

  //
  // = Glyph =========================================================================================================
  // An individual character glyph, used for packing
  //
  class Glyph
  {
    FT_Library       ft; // ugh
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
      ft = slot -> library;

      index = new_index;

      Rk::zero_out (bitmap);

      auto error = FT_Bitmap_Convert (ft, &slot -> bitmap, &bitmap, 1);
      if (error)
        Rk::raise () << "Error copying glyph bitmap for glyph #" << index;

      metrics = slot -> metrics;

      max_side = std::max (width (), height ());
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
    if (first == end)
      throw std::runtime_error ("Co-Font: make_atlas - no glyphs to pack");

    threshold = Rk::clamp (threshold, 0.0f, 1.0f);

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
  // = map_char ========================================================================================================
  //
  template <typename CharMap>
  void map_char (FT_Face& face, char32 cp, CharMap& char_map, std::map <uint, uint>& remaps, uint& glyph_count)
  {
    uint freetype_index = FT_Get_Char_Index (face, cp);

    // No glyph for this character
    if (freetype_index == 0)
      freetype_index = FT_Get_Char_Index (face, 0xfffd);

    // Character uses a glyph - do we already have it?
    auto remap = remaps.find (freetype_index);
    if (remap != remaps.end ())
    {
      // We already have the glyph remapped
      char_map [cp] = remap -> second;
    }
    else
    {
      // We need to remap the glyph
      remaps [freetype_index] = glyph_count;
      char_map [cp] = glyph_count;
      glyph_count++;
    }
  }

  //
  // construct
  //
  void FontImpl::construct (std::shared_ptr <FontImpl>& self, WorkQueue& queue, LoadContext& ctx)
  {
    Rk::Image image;
    uint approx_height;
    float threshold = 0.95f;

    // Load FreeType
    FT_Library ft;
    auto error = FT_Init_FreeType (&ft);
    if (error)
      throw std::runtime_error ("Co-Font: create_font - FT_Init_FreeType failed");
    
    auto ft_guard = Rk::guard (
      [&ft] { FT_Done_FreeType (ft); }
    );

    // Load face
    FT_Face face;
    Rk::ShortString <512> path_buf = path;
    error = FT_New_Face (ft, path_buf.c_str (), 0, &face);
    if (error)
      throw std::runtime_error ("Co-Font: create_font - FT_New_Face failed");

    #ifdef CO_DEBUG

    if (face -> num_fixed_sizes != 0)
    {
      auto lock = ctx.log.lock ();
      lock << "i Strike sizes for \"" << path << "\":\n";
    
      for (uint i = 0; i != face -> num_fixed_sizes; i++)
      {
        auto& size = face -> available_sizes [i];
        lock << "    " << (size.x_ppem >> 6) << "x" << (size.y_ppem >> 6) << '\n';
      }
    }

    #endif

    // Set size
    if (mode == fontsize_points)
    {
      uint dpi = 96;
      error = FT_Set_Char_Size (face, 0, size << 6, dpi, dpi);
      if (error)
        throw std::runtime_error ("Co-Font: create_font - FT_Set_Char_Size failed");
      approx_height = (size * dpi) / 72;
    }
    else
    {
      error = FT_Set_Pixel_Sizes (face, 0, size);
      if (error)
        throw std::runtime_error ("Co-Font: create_font - FT_Set_Char_Size failed");
      approx_height = size;
    }

    // Map codepoints to glyph indices
    std::map <uint, uint> glyph_remaps; 
    
    uint glyph_count = 0;

    for (auto range = code_ranges.begin (); range != code_ranges.end (); range++)
    {
      for (char32 cp = range -> begin; cp != range -> end; cp++)
        map_char (face, cp, char_map, glyph_remaps, glyph_count);
    }

    // Discover kerning pairs
    for (auto left = glyph_remaps.begin (); left != glyph_remaps.end (); left++)
    {
      for (auto right = glyph_remaps.begin (); right != glyph_remaps.end (); right++)
      {
        FT_Vector kerning;
        auto error = FT_Get_Kerning (face, left -> first, right -> first, 0, &kerning);
        if (kerning.x || kerning.y)
        {
          u64 key = (u64 (left -> second) << 32) | right -> second;
          kerning_pairs [key] = v2i (kerning.x >> 6, kerning.y >> 6);
        }
      }
    }

    // Make sure we have U+FFFD REPLACEMENT CHARACTER
    map_char (face, 0xfffd, char_map, glyph_remaps, glyph_count);

    // Render necessary glyphs
    std::vector <Glyph> glyphs;
    glyphs.resize (glyph_count);

    for (auto remap = glyph_remaps.begin (); remap != glyph_remaps.end (); remap++)
    {
      error = FT_Load_Glyph (face, remap -> first, FT_LOAD_NO_AUTOHINT);
      if (error)
        throw std::runtime_error ("Co-Font: create_font - FT_Load_Glyph failed");

      if (face -> glyph -> format != FT_GLYPH_FORMAT_BITMAP)
      {
        error = FT_Render_Glyph (face -> glyph, FT_RENDER_MODE_NORMAL);
        if (error)
          throw std::runtime_error ("Co-Font: create_font - FT_Render_Glyph failed");
      }

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

    // Render the packed glyphs to an image
    image.pixel_type   = Rk::i_8;
    image.row_stride   = image.width;
    image.bottom_up    = false;
    image.pixel_stride = 1;
    image.allocate ();
    for (auto ptr = image.data; ptr != image.data + image.size (); ptr++)
      *ptr = 255;

    for (auto iter = glyphs.begin (); iter != glyphs.end (); iter++)
      iter -> blit (image);

    // Dump packed glyph map
    #ifdef CO_DEBUG

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

    TGAHeader head = {
      0,
      1, 1,
      0, 256, 24,
      0, 0,
      image.width, image.height,
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

    Rk::File dump (path + ".dump.tga", Rk::File::open_replace_or_create);
    dump.put (head)
        .put (palette)
        .write (image.data, image.size ());

    #endif // CO_DEBUG

    // Re-sort glyphs by private index for use and caching
    std::sort (
      glyphs.begin (),
      glyphs.end   (),
      [] (const Glyph& a, const Glyph& b)
      {
        return a.index < b.index;
      }
    );

    // Build glyph metrics
    GlyphMetrics::Vector new_metrics (glyph_count);
    uint dest_index = 0;
    for (auto source = glyphs.begin (); source != glyphs.end (); source++)
      new_metrics [dest_index++] = source -> get_metrics ();

    // Upload texture
    auto tex = ctx.rc.create_tex_rectangle (queue, texformat_a8, image.width, image.height, texwrap_clamp, image.data, image.size ());
    
    auto lock = mutex.get_lock ();
    font_height = approx_height;
    this -> mets = std::move (new_metrics);
    this -> tex  = std::move (tex);
  }

  //
  // retrieve
  //
  void FontImpl::retrieve (TexImage::Ptr& tex, GlyphMetrics::Vector& mets, uint& font_height) const
  {
    auto lock = mutex.get_lock ();
    tex         = std::move (this -> tex);
    mets        = std::move (this -> mets);
    font_height = this -> font_height;
  }

  //
  // translate_codepoint
  //
  Character FontImpl::translate_codepoint (char32 cp, u32& prev_index) const
  {
    auto iter = char_map.find (cp);

    if (iter == char_map.end () && cp != 0xfffd)
      return translate_codepoint (0xfffd, prev_index);

    v2i kerning (0, 0);
    if (prev_index && iter -> second)
    {
      u64 key = (u64 (prev_index) << 32) | iter -> second;
      auto kern_iter = kerning_pairs.find (key);
      if (kern_iter != kerning_pairs.end ())
        kerning = kern_iter -> second;
    }

    prev_index = iter -> second;

    return Character (iter -> second, kerning.x);
  }

  //
  // translate_codepoints
  //
  void FontImpl::translate_codepoints (const char32* begin, const char32* end, Character* chars) const
  {
    if (!begin || !end || !chars)
      throw std::invalid_argument ("Co-Font: translate_codepoints - null pointer");

    Rk::check_range (begin, end);

    u32 prev_index = 0;

    while (begin != end)
      *chars++ = translate_codepoint (*begin++, prev_index);
  }

  //
  // translate_utf16
  //
  void FontImpl::translate_utf16 (const char16* begin, const char16* end, Character* chars) const
  {
    if (!begin || !end || !chars)
      throw std::invalid_argument ("Co-Font: translate_codepoints - null pointer");

    Rk::check_range (begin, end);

    u32 prev_ft_index = 0;

    char32 cp = 0;
    bool in_pair = false;

    while (begin != end)
    {
      char16 unit = *begin++;

      if (unit >= 0xd800 && unit < 0xe000)
      {
        if (unit < 0xdc00) // lead surrogate
        {
          if (in_pair)
            throw std::runtime_error ("Co-Font: translate_utf16 - invalid string - double lead surrogate");

          cp = unit - 0xd800;
          in_pair = true;
        }
        else
        {
          if (!in_pair)
            throw std::runtime_error ("Co-Font: translate_utf16 - invalid_string - unpaired trail surrogate");

          cp = (cp << 10) | (unit - 0xdc00);
          in_pair = false;
        }
      }
      else
      {
        cp = unit;
      }

      *chars++ = translate_codepoint (cp, prev_ft_index);
    }
  }

  template <typename Iter>
  FontImpl::FontImpl (
    Rk::StringRef new_path,
    uint          new_size,
    FontSizeMode  new_mode,
    Iter          ranges,
    Iter          end,
    uint          new_index
  ) :
    path        (/*"Fonts/" +*/ new_path.string ()),
    size        (new_size),
    index       (new_index),
    mode        (new_mode),
    code_ranges (ranges, end)
  { }
  
  template <typename Iter>
  static Font::Ptr FontImpl::create (
    WorkQueue&     queue,
    Rk::StringRef  path,
    uint           size,
    FontSizeMode   mode,
    Iter           ranges,
    Iter           end,
    uint           index)
  {
    return queue.gc_construct (new FontImpl (path, size, mode, ranges, end, index));
  }

  //
  // = FactoryImpl =====================================================================================================
  //
  extern const RangeSet repetoire_wgl4;
  extern const RangeSet repetoire_bmp;

  class FactoryImpl :
    public FontFactory,
    public ResourceFactory <std::string, FontImpl>
  {
    Log&       log;
    WorkQueue& queue;

    virtual Font::Ptr create (
      Rk::StringRef    path,
      uint             size,
      FontSizeMode     mode,
      const CodeRange* ranges,
      const CodeRange* end,
      uint             index)
    {
      Rk::StringOutStream lookup;
      lookup << path << '-' << index << '-' << size;
      if (mode == fontsize_points)
        lookup << "pt";
      else
        lookup << "px";

      auto key = lookup.string ();

      auto ptr = find (key);
      if (!ptr)
      {
        ptr = FontImpl::create (queue, path, size, mode, ranges, end, index);
        add (key, ptr);
      }
      return std::move (ptr);
    }

    virtual RangeSet get_repetoire (Rk::StringRef name)
    {
      //log () << "Co::Font: get_repetoire - " << name << '\n';

      if (name == "WGL4")
        return repetoire_wgl4;
      else if (name == "BMP")
        return repetoire_bmp;
      else
        return RangeSet (nullptr, nullptr);
    }

  public:
    FactoryImpl (Log& log, WorkQueue& queue) :
      log   (log),
      queue (queue)
    { }
    
  };

  class Root :
    public FontRoot
  {
    virtual FontFactory::Ptr create_factory (Log& log, WorkQueue& queue)
    {
      return std::make_shared <FactoryImpl> (log, queue);
    }

  };

  RK_MODULE (Root);

} // namespace Co
