/***************************************************************************/
/*                                                                         */
/*  ftoption.h                                                             */
/*                                                                         */
/*    User-selectable configuration macros (specification only).           */
/*                                                                         */
/*  Copyright 1996-2011 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTOPTION_H__
#define __FTOPTION_H__


#include <ft2build.h>


FT_BEGIN_HEADER

#undef FT_CONFIG_OPTION_FORCE_INT64

/* #define FT_CONFIG_OPTION_NO_ASSEMBLER */

#define FT_CONFIG_OPTION_INLINE_MULFIX

#define FT_CONFIG_OPTION_USE_LZW

#define FT_CONFIG_OPTION_USE_ZLIB

/* #define FT_CONFIG_OPTION_SYSTEM_ZLIB */

/* #define FT_CONFIG_OPTION_USE_BZIP2 */

/* #define FT_CONFIG_OPTION_DISABLE_STREAM_SUPPORT */

#ifdef FT2_BUILD_LIBRARY
  #define FT_EXPORT(x)      extern __declspec (dllexport) x
  #define FT_EXPORT_DEF(x)  __declspec (dllexport) x
#else
  #define FT_EXPORT(x)      extern __declspec (dllimport) x
  #define FT_EXPORT_DEF(x)  __declspec (dllimport) x
#endif

#define FT_CONFIG_OPTION_POSTSCRIPT_NAMES

/* Needed for Unicode mapping of Type I fonts */
/* #define FT_CONFIG_OPTION_ADOBE_GLYPH_LIST */

/* #define FT_CONFIG_OPTION_MAC_FONTS */

#define FT_RENDER_POOL_SIZE  16384L

#define FT_MAX_MODULES  32

/* #define FT_DEBUG_LEVEL_ERROR */
/* #define FT_DEBUG_LEVEL_TRACE */
/* #define FT_DEBUG_AUTOFIT */
/* #define FT_DEBUG_MEMORY */

#undef FT_CONFIG_OPTION_USE_MODULE_ERRORS

/* #define FT_CONFIG_OPTION_PIC */

#define TT_CONFIG_OPTION_EMBEDDED_BITMAPS

#define TT_CONFIG_OPTION_POSTSCRIPT_NAMES

#define TT_CONFIG_OPTION_SFNT_NAMES

#define TT_CONFIG_CMAP_FORMAT_0
#define TT_CONFIG_CMAP_FORMAT_2
#define TT_CONFIG_CMAP_FORMAT_4
#define TT_CONFIG_CMAP_FORMAT_6
#define TT_CONFIG_CMAP_FORMAT_8
#define TT_CONFIG_CMAP_FORMAT_10
#define TT_CONFIG_CMAP_FORMAT_12
#define TT_CONFIG_CMAP_FORMAT_13
#define TT_CONFIG_CMAP_FORMAT_14

#define TT_CONFIG_OPTION_BYTECODE_INTERPRETER
/* #define TT_CONFIG_OPTION_UNPATENTED_HINTING */
#define TT_CONFIG_OPTION_INTERPRETER_SWITCH

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
  #define  TT_USE_BYTECODE_INTERPRETER
  #undef   TT_CONFIG_OPTION_UNPATENTED_HINTING
#elif defined TT_CONFIG_OPTION_UNPATENTED_HINTING
  #define  TT_USE_BYTECODE_INTERPRETER
#endif

#undef TT_CONFIG_OPTION_COMPONENT_OFFSET_SCALED

/* Bitmap strikes */
#define TT_CONFIG_OPTION_BDF

#define T1_MAX_DICT_DEPTH  5
#define T1_MAX_SUBRS_CALLS  16
#define T1_MAX_CHARSTRINGS_OPERANDS  256

/* For kerning */
#undef T1_CONFIG_OPTION_NO_AFM

#undef T1_CONFIG_OPTION_NO_MM_SUPPORT

#define AF_CONFIG_OPTION_CJK
#define AF_CONFIG_OPTION_INDIC

/* #define AF_CONFIG_OPTION_USE_WARPER */

#undef FT_CONFIG_OPTION_OLD_INTERNALS


FT_END_HEADER


#endif /* __FTOPTION_H__ */


/* END */
