//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxFontFactory.hpp>
#include <Co/IxFont.hpp>

// Uses
#include <Co/IxLoadContext.hpp>

#include <Rk/ShortString.hpp>
#include <Rk/Expose.hpp>

#include <unordered_map>

namespace Co
{
  //
  // = Font ============================================================================================================
  //
  class Font :
    public IxFont
  {
    Rk::ShortString <512> path;
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
      image -> destroy ();
    }

    virtual void load (IxRenderContext& rc)
    {
      

      ready = true;
    }

  public:
    Font (IxLoadContext& context, Rk::StringRef new_path)
    {
      ref_count = 1;
      path = context.get_game_path ();
      path += "Fonts/";
      path += new_path;
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

    virtual IxFont* create (IxLoadContext& context, Rk::StringRef path)
    {
      IxFont* font;

      auto iter = cache.find (path);

      if (iter != cache.end ())
      {
        font = iter -> second;
        font -> acquire ();
      }
      else
      {
        font = new Font (context, path);
        cache.insert (CacheType::value_type (path, font));
      }
      
      return font;
    }

  } factory;

  IX_EXPOSE (void** out, u64 ixid)
  {
    Rk::expose <IxFontFactory> (factory, ixid, out);
  }

} // namespace Co
