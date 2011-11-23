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
    FontPack pack;
    
    // Management
    long ref_count;

    virtual void acquire ()
    {
      _InterlockedIncrement (&ref_count);
    }

    virtual void release ()
    {
      _InterlockedDecrement (&ref_count);
    }

    virtual void dispose ()
    {
      delete this;
    }

    ~Font ()
    {
      pack -> destroy ();
    }

    virtual void load (IxRenderContext& rc)
    {
      CodeRange ranges [1] = {
        { 0x0020, 0x007f } // ASCII printable
      };

      Rk::Image image;
      pack = FontPack (image, path, index, size, mode, ranges, ranges + 1, 0.95f);

      tex = rc.create_tex_image (1, false);
      tex -> load_map (0, image.data, tex_i8, image.width, image.height, image.size ());

      ready = true;
    }

  public:
    Font (IxLoadContext& context, Rk::StringRef new_path, uint new_size, FontSizeMode new_mode, uint new_index)
    {
      ref_count = 1;
      path = context.get_game_path ();
      path += "Fonts/";
      path += new_path;
      size = new_size;
      mode = new_mode;
      index = new_index;
      image = 0;
      context.load (this);
    }

    bool dead () const
    {
      return ref_count == 0;
    }

  }; // class Font

  //
  // = FontFactory =====================================================================================================
  //
  class FontFactory :
    public IxFontFactory
  {
    typedef std::unordered_map <Rk::ShortString <512>, IxFont*> CacheType;
    CacheType cache;

    virtual IxFont* create (IxLoadContext& context, Rk::StringRef path, uint size, FontSizeMode mode, uint index)
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

  } factory;

  IX_EXPOSE (void** out, u64 ixid)
  {
    Rk::expose <IxFontFactory> (factory, ixid, out);
  }

} // namespace Co
