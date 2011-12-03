//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/TextureFile.hpp>

#include <Rk/ShortString.hpp>
#include <Rk/Exception.hpp>
#include <Rk/ImageFile.hpp>
#include <Rk/DXTCodec.hpp>
#include <Rk/Console.hpp>
#include <Rk/File.hpp>

#include <vector>

using namespace Co;

static void box_filter (u8* dest, const u8* src_in, uint src_row_stride, uint factor)
{
  const u8* src = src_in;

  if (factor != 1)
  {
    uint accum [3] = { 0, 0, 0 };
    
    for (uint y = 0; y != factor; y++)
    {
      for (uint x = 0; x != factor; x++)
      {
        accum [0] += src [x * 3 + 0];
        accum [1] += src [x * 3 + 1];
        accum [2] += src [x * 3 + 2];
      }
      
      src += src_row_stride;
    }
    
    uint factor_squared = factor * factor;
    float recip_factor_squared = 1.0f / float (factor_squared);
    dest [0] = u8 (accum [2] * recip_factor_squared);
    dest [1] = u8 (accum [1] * recip_factor_squared);
    dest [2] = u8 (accum [0] * recip_factor_squared);
    dest [3] = 255;
  }
  else
  {
    dest [0] = src [2];
    dest [1] = src [1];
    dest [2] = src [0];
    dest [3] = 255;
  }
}

static void build_mipmap (Rk::Image& in, u8*& out, uint level)
{
  const uint factor = 1 << level;

  u32 src_block_row_off = in.size () - in.row_stride * 4 * factor;
  
  // Filter and compress one block at a time
  for (uint block_y = 0; block_y != (in.height >> level) / 4; block_y++)
  {
    u32 src_block_off = src_block_row_off;

    for (uint block_x = 0; block_x != (in.width  >> level) / 4; block_x++)
    {
      // Filter block
      u8 block [64];
      uint block_off = 0;

      u32 src_pixel_row_off = src_block_off + in.row_stride * 3 * factor;

      for (uint block_pixel_y = 0; block_pixel_y != 4; block_pixel_y++)
      {
        u32 src_pixel_off = src_pixel_row_off;

        for (uint block_pixel_x = 0; block_pixel_x != 4; block_pixel_x++)
        {
          box_filter (block + block_off, in.data + src_pixel_off, in.row_stride, factor);
          block_off += 4;
          src_pixel_off += in.pixel_stride * factor;
        }

        src_pixel_row_off -= in.row_stride * factor;
      }

      Rk::dxt_encode_block (out, block, false);
      out += 8;

      src_block_off += in.pixel_stride * 4 * factor;
    }

    src_block_row_off -= in.row_stride * 4 * factor;
  }
}

static void write_texture (Rk::File& out_file, u16 mipmap_count, TexFormat format, u16 width, u16 height, const u8* data, u32 data_size)
{
  Rk::std_err << "Output format is " << tex_format_strings [format] << '\n';

  TextureHeader header = {
    0x20111202,
    mipmap_count,
    format,
    width,
    height
  };

  out_file.write (Co::texture_magic, 8);

  out_file.write ("HEAD", 4);
  out_file.put (u32 (sizeof (header)));
  out_file.put (header);

  out_file.write ("DATA", 4);
  out_file.put (u32 (data_size));
  out_file.write (data, data_size);

  out_file.write ("END.", 4);
  out_file.put (u32 (0));
}

static void convert_texture (Rk::Image& in_image, Rk::File& out_file, bool verbose, /*bool compress,*/ bool mipmap)
{
  static const char* format_strings [11] = {
    "unknown",
    "rgb565",
    "bgr565",
    "rgb888",
    "bgr888",
    "rgba8888",
    "argb8888",
    "bgra8888",
    "abgr8888",
    "i8",
    "i1"
  };

  if (verbose)
    Rk::std_err << "Source image is " << in_image.width << 'x' << in_image.height << ":" << format_strings [in_image.pixel_type] << '\n';

  if (in_image.pixel_type != Rk::Image::bgr888)
    throw Rk::Exception ("Only bgr888 source images are currently supported");

  if (in_image.width > 65536 || in_image.height > 65536)
    throw Rk::Exception ("Source image may not be wider or taller than 65536 pixels");

  if (in_image.width % 4 || in_image.height % 4)
    throw Rk::Exception ("Dimensions of source image must be multiples of 4");
  
  if (mipmap)
  {
    u32 mipmap_blocks_w = in_image.width  / 4,
        mipmap_blocks_h = in_image.height / 4,
        limit = Rk::minimum (mipmap_blocks_w, mipmap_blocks_h),
        mipmap_size = mipmap_blocks_w * mipmap_blocks_h * 8,
        data_size = 0;
    u16 mipmap_count = 0;
    
    while (limit)
    {
      data_size += mipmap_size;
      mipmap_count++;
      mipmap_size /= 4;
      limit /= 2;
    }

    Rk::std_err << "Generating " << mipmap_count << " mipmaps\n";

    std::vector <u8> buffer (data_size);

    u8* ptr = buffer.data ();
    for (uint level = 0; level != mipmap_count; level++)
      build_mipmap (in_image, ptr, level);

    write_texture (out_file, mipmap_count, texformat_dxt1, in_image.width & 0xffff, in_image.height & 0xffff, buffer.data (), data_size);
  }
  else
  {
    /*if (in_image.bottom_up)
    {
      // Flip image
      for (uint y = 0; y != in_image.height / 2; y++)
      {
        u8* high = in_image.data + in_image.row_stride * y;
        u8* low  = in_image.data + in_image.row_stride * (in_image.height - y - 1);
        for (uint x = 0; x != in_image.width * in_image.pixel_stride; x++)
          std::swap (*high++, *low++);
      }
    }*/

    write_texture (out_file, 1, texformat_bgr888, in_image.width & 0xffff, in_image.height & 0xffff, in_image.data, in_image.size ());
  }
}

int main (int arg_count, const char** args) try
{
  Rk::StringRef         in_path;
  Rk::ShortString <512> out_path;
  bool                  verbose  = false,
                        //compress = false,
                        mipmap   = false;

  for (uint arg_index = 1; arg_index != arg_count; arg_index++)
  {
    Rk::StringRef arg = args [arg_index];

    if (arg == "-o")
    {
      if (out_path)
        throw Rk::Exception ("Only one destination file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception ("Expected destination file path after \"-o\"");
      out_path = args [arg_index];
    }
    /*else if (arg == "-c")
    {
      compress = true;
    }*/
    else if (arg == "-m")
    {
      mipmap = true;
    }
    else if (arg == "-v" || arg == "--verbose")
    {
      verbose = true;
    }
    else if (arg == "-h" || arg == "--help" || arg == "/?")
    {
      Rk::std_err << "\nCo-Texture-Tool (c) 2011 Rk\n\n"
                  << "  Converts an image file to a Cobalt texture, optionally generating mipmaps\n"
                  << "  and compressing it in the process.\n\n"
                  << "Usage:\n"
                  << "  " << args [0] << " [-i] source-path [-o output-path] [-c] [-m] [-v]\n"
                  << "    Converts the image file at source-path to a Cobalt texture.\n\n"
                  << "  " << args [0] << " {-h|--help|/?}\n"
                  << "    Prints this message\n\n"
                  << "Options:\n"
                  << "  -i source-path      Uses source-path as the path to the image file to be\n"
                  << "                      converted.\n"
                  << "  -o output-path      Uses output-path as the path to the texture file to\n"
                  << "                      be created. If this file exists, it is overwritten\n"
                  << "                      without confirmation. If this option is not specified,\n"
                  << "                      then a file with a name similar to the source file in\n"
                  << "                      the current directory is used.\n"
                /*<< "  -c                  Compresses the texture with DXT as appropriate.\n"*/
                  << "  -m                  Generates mipmaps using a box filter.\n"
                  << "  -v                  Verbose mode. Prints conversion information to the\n"
                  << "                      standard error stream.\n";
      return 0;
    }
    else
    {
      if (arg == "-i")
      {
        if (++arg_index == arg_count)
          throw Rk::Exception ("Expected source file path after \"-i\"");
      }
      else if (arg [0] == '-')
      {
        throw Rk::Exception () << "Unrecognized option \"" << arg << "\"";
      }
      
      if (in_path)
        throw Rk::Exception ("Only one source file may be specified");

      in_path = args [arg_index];
    }
    
  }

  if (!in_path)
    throw Rk::Exception ("No input file specified");

  if (!out_path)
  {
    uptr dot = in_path.reverse_find ('.');
    if (dot == in_path.nowhere)
      dot = in_path.length ();
    out_path = in_path.range (0, dot);
    out_path += ".cotexture";
  }

  if (verbose)
    Rk::std_err << "Converting " << in_path << " to " << out_path << '\n';

  Rk::ImageFile in_file  (in_path);
  Rk::File      out_file (out_path, Rk::File::open_replace_or_create);

  convert_texture (in_file, out_file, verbose, /* compress, */ mipmap);

  return 0;
}
catch (const std::exception& e)
{
  Rk::std_err << e.what () << '\n';
  return 1;
}
catch (...)
{
  Rk::std_err << "Exception\n";
  return 1;
}
