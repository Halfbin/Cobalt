//
// This file is in the public domain
//

#include <Co/Font.hpp>

namespace Co
{
  // Defines half-open ranges of Unicode codepoints which
  // specify (essentially all of) the Unicode Basic Multilingual Plane
  static const CodeRange ranges [] = {
    { 0x0020, 0xfffe }, 
  };

  // consts are internal by default
  // why.jpg
  extern const RangeSet repetoire_bmp (ranges);

}
