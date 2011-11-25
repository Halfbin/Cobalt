//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXFONT
#define CO_H_IXFONT

// Implements
#include <Co/IxResource.hpp>

namespace Co
{
  class IxTexImage;

  struct GlyphMetrics
  {
    u32 x,
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

  class IxFont : 
    public IxResource
  {
  protected:
    IxTexImage*   tex;
    GlyphMetrics* metrics;

    IxFont () :
      tex     (0),
      metrics (0)
    { }
    
  public:
    typedef Rk::IxSharedPtr <IxFont> Ptr;

    virtual void translate_codepoints (const char32* codepoints, const char32* end, u32* indices) = 0;
    
    void translate_codepoints (const char32* codepoints, uint count, u32* indices)
    {
      translate_codepoints (codepoints, codepoints + count, indices);
    }

    const GlyphMetrics& get_metrics (uint index)
    {
      return metrics [index];
    }

    IxTexImage* get_image ()
    {
      return tex;
    }

    bool ready () const
    {
      return IxResource::ready;
    }

  }; // class IxFont

} // namespace Co

#endif
