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

#include <Rk/ShortString.hpp>
#include <Rk/Exception.hpp>
#include <Rk/Expose.hpp>

#include <unordered_map>
#include <algorithm>
#include <vector>

#include "Packer.hpp"

namespace Co
{
  namespace
  {
    //
    // = Font ============================================================================================================
    //
    class Font :
      public IxFont
    {
      // Parameters
      Rk::ShortString <512> path;
      uint                  size,
                            index;
      FontSizeMode          mode;

      // Data
      CharMap char_map;
    
      // Management
      long ref_count;

      virtual void acquire ();
      virtual void release ();
      virtual void dispose ();

      ~Font ();

      virtual void load (IxRenderContext& rc);

      virtual void translate_codepoints (const char32* begin, const char32* end, u32* indices);

    public:
      Font (IxLoadContext& context, Rk::StringRef new_path, uint new_size, FontSizeMode new_mode, uint new_index);

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

    Font::~Font ()
    {
      tex -> destroy ();
      delete [] metrics;
    }

    void Font::load (IxRenderContext& rc)
    {
      CodeRange ranges [1] = {
        { 0x0020, 0x007f } // ASCII printable
      };

      Rk::Image image;
      auto ptr = create_font (char_map, image, path, index, size, mode, ranges, ranges + 1, 0.95f);
      metrics = ptr.release ();

      tex = rc.create_tex_image (1, false);
      tex -> load_map (0, image.data, tex_i8, image.width, image.height, image.size ());

      ready = true;
    }

    void Font::translate_codepoints (const char32* begin, const char32* end, u32* indices)
    {
      if (!begin || !end || !indices)
        throw Rk::Exception ("Co-Font: IxFont::translate_codepoints - null pointer");

      if (end > begin)
        throw Rk::Exception ("Co-Font: IxFont::translate_codepoints - invalid range");

      while (begin != end)
      {
        auto iter = char_map.find (*begin++);
        if (iter != char_map.end ())
          *indices++ = iter -> second;
        else
          *indices++ = 0;
      }
    }

    Font::Font (IxLoadContext& context, Rk::StringRef new_path, uint new_size, FontSizeMode new_mode, uint new_index)
    {
      ref_count = 1;
      path = context.get_game_path ();
      path += "Fonts/";
      path += new_path;
      size = new_size;
      mode = new_mode;
      index = new_index;
      context.load (this);
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

      virtual IxFont* create (IxLoadContext& context, Rk::StringRef path, uint size, FontSizeMode mode, uint index);

    public:
      void expose (void** out, u64 ixid);

    } factory;

    IxFont* FontFactory::create (IxLoadContext& context, Rk::StringRef path, uint size, FontSizeMode mode, uint index)
    {
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
        font = new Font (context, path, size, mode, index);
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
