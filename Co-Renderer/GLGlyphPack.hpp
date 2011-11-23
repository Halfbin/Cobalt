//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//
/*
#ifndef CO_GLRENDERER_H_GLGLYPHPACK
#define CO_GLRENDERER_H_GLGLYPHPACK

// Implements
#include <Co/IxGlyphPack.hpp>

// Uses
#include "GLTexImage.hpp"

namespace Co
{
  class GLGlyphPack :
    public IxGlyphPack,
    public GLTexImage
  {
    GlyphMapping* mappings;

    ~GLGlyphPack ();
    virtual void destroy ();

  public:
    GLGlyphPack (const u8* atlas, uint width, uint height, const GlyphMapping* new_mappings, uint glyph_count);
    
    template <typename InIter, typename OutIter>
    void map_glyphs (InIter in, InIter end, OutIter out)
    {
      while (in != end)
        *out++ = mappings [*in++];
    }

  };

}

#endif
*/