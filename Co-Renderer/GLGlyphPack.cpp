//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//
/*
// Implements
#include "GLGlyphPack.hpp"

namespace Co
{
  GLGlyphPack::~GLGlyphPack ()
  {
    delete [] mappings;
  }

  void GLGlyphPack::destroy ()
  {
    delete this;
  }

  GLGlyphPack::GLGlyphPack (const u8* atlas, uint width, uint height, const GlyphMapping* new_mappings, uint glyph_count) :
    GLTexImage (1, false)
  {
    load_map (0, atlas, tex_i8, width, height);
    mappings = new GlyphMapping [glyph_count];
    for (uint i = 0; i != glyph_count; i++)
      mappings [i] = new_mappings [i];
  }
  
}
*/