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

struct Options
{
  bool compress,
       mipmap,
       verbose;

  Options () :
    compress (true),
    mipmap   (true),
    verbose  (false)
  { }
  
} options;

static void box_filter (u8* dest, const u8* src_in, uint src_row_stride, uint factor)
{
  const u8* src = src_in;

  if (factor != 1)
  {
    // u32 can only manage level <= 12 (4096:1)
    // because (8192^2 * 2^8) > 2^32
    u64 accum [3] = { 0, 0, 0 };
    
    for (uint y = 0; y != factor; y++)
    {
      for (uint x = 0; x != factor; x++)
      {
        accum [0] += u64 (src [x * 3 + 0]);
        accum [1] += u64 (src [x * 3 + 1]);
        accum [2] += u64 (src [x * 3 + 2]);
      }
      
      src += src_row_stride;
    }
    
    uint factor_squared = factor * factor;
    double recip_factor_squared = 1.0 / double (factor_squared);
    dest [0] = u8 (double (accum [2]) * recip_factor_squared);
    dest [1] = u8 (double (accum [1]) * recip_factor_squared);
    dest [2] = u8 (double (accum [0]) * recip_factor_squared);
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

static u8* build_raw_mipmap (Rk::Image& in, u8* out, uint level)
{
  const uint factor = 1 << level;

  uptr y_off = 0;

  for (uint y = 0; y != in.height >> level; y++)
  {
    uptr x_off = 0;

    for (uint x = 0; x != in.width >> level; x++)
    {
      box_filter (out, in.data + y_off + x_off, in.row_stride, factor);
      out += 4;
      x_off += in.pixel_stride * level;
    }

    y_off += in.row_stride * level;
  }

  return out;
}

static u8* build_dxt_mipmap (Rk::Image& in, u8* out, uint level)
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

  return out;
}

static void write_texture (Rk::File& out_file, u8 mipmap_count, u8 flags, TexFormat format, u16 width, u16 height, const u8* data, u32 data_size)
{
  if (options.verbose)
    Rk::std_err << "Output format is " << tex_format_strings [format] << '\n';

  TextureHeader header = {
    0x20111206,
    flags,
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

static const char* const format_strings [11] = {
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

static void check_image (Rk::Image& image)
{
  if (image.pixel_type != Rk::Image::bgr888)
    throw Rk::Exception ("Only bgr888 source images are currently supported");

  if (image.width > 65536 || image.height > 65536)
    throw Rk::Exception ("Source images may not be wider or taller than 65536 pixels");

  if (image.width % 4 || image.height % 4)
    throw Rk::Exception ("Dimensions of source images must be multiples of 4");
}

static void compute_output_info (u32 width, u32 height, u8& mipmap_count, u32& data_size)
{
  u32 limit,
      mipmap_size;

  data_size    = 0;
  mipmap_count = 0;

  if (options.compress)
  {
    u32 mipmap_blocks_w = width  / 4,
        mipmap_blocks_h = height / 4;
    limit = std::min (mipmap_blocks_w, mipmap_blocks_h);
    mipmap_size = mipmap_blocks_w * mipmap_blocks_h * 8;
  }
  else
  {
    limit = std::min (width / 4, height / 4);
    mipmap_size = width * height * 4;
  }

  for (;;)
  {
    data_size += mipmap_size;
    mipmap_count++;
    mipmap_size /= 4;
    if (!options.mipmap || limit % 2) break;
    limit /= 2;
  }

  if (options.mipmap && options.verbose)
    Rk::std_err << "Generating " << mipmap_count << " mipmaps\n";
}

static u8* build_mipmaps (Rk::Image& in_image, u8 mipmap_count, u8* out)
{
  for (uint level = 0; level != mipmap_count; level++)
  {
    if (options.compress)
      out = build_dxt_mipmap (in_image, out, level);
    else
      out = build_raw_mipmap (in_image, out, level);
  }

  return out;
}

static void convert_texture (Rk::Image& in_image, Rk::File& out_file)
{
  if (options.verbose)
    Rk::std_err << "Source image is " << in_image.width << 'x' << in_image.height << ":" << format_strings [in_image.pixel_type] << '\n';

  check_image (in_image);
  
  u8  mipmap_count;
  u32 data_size;
  compute_output_info (in_image.width, in_image.height, mipmap_count, data_size);

  std::unique_ptr <u8 []> buffer (new u8 [data_size]);
  build_mipmaps (in_image, mipmap_count, buffer.get ());

  write_texture (
    out_file,
    mipmap_count,
    0,
    options.compress ? texformat_dxt1 : texformat_rgba8888,
    in_image.width  & 0xffff,
    in_image.height & 0xffff,
    buffer.get (),
    data_size
  );

  out_file.delete_on_close (false);
}

void convert_cubemap (Rk::ImageFile (&in_images) [6], Rk::File& out_file)
{
  if (options.verbose)
  {
    static const char* const face_strings [6] = { "front:  ", "back:   ", "left:   ", "right:  ", "top:    ", "bottom: " };
    Rk::std_err << "Source images are:\n";
    for (uint i = 0; i != 6; i++)
    {
      Rk::std_err << "  " << face_strings [i]
                  << in_images [i].width << 'x' << in_images [i].height << ":"
                  << format_strings [in_images [i].pixel_type] << '\n';
    }
  }

  for (uint i = 0; i != 6; i++)
    check_image (in_images [i]);

  uint width  = in_images [0].width,
       height = in_images [0].height;
  auto type   = in_images [0].pixel_type;

  for (uint i = 1; i != 6; i++)
  {
    if (
      in_images [i].width      != width  ||
      in_images [i].height     != height ||
      in_images [i].pixel_type != type)
    {
      throw Rk::Exception ("Source images differ in format or dimensions");
    }
  }

  u8  mipmap_count;
  u32 data_size;
  compute_output_info (width, height, mipmap_count, data_size);

  std::unique_ptr <u8 []> buffer (new u8 [data_size * 6]);
  u8* ptr = buffer.get ();
  for (uint i = 0; i != 6; i++)
    ptr = build_mipmaps (in_images [i], mipmap_count, ptr);

  write_texture (
    out_file,
    mipmap_count,
    texflag_cube_map,
    options.compress ? texformat_dxt1 : texformat_rgba8888,
    width & 0xffff,
    height & 0xffff,
    buffer.get (),
    data_size * 6
  );

  out_file.delete_on_close (false);
}

int main (int arg_count, const char** args) try
{
  Rk::StringRef         in_path,
                        face_paths [6];
  Rk::ShortString <512> out_path;
  bool                  cubemap = false;

  for (uint arg_index = 1; arg_index != arg_count; arg_index++)
  {
    Rk::StringRef arg = args [arg_index];

    if (arg == "-o")
    {
      if (out_path)
        throw Rk::Exception ("Only one destination file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected destination file path after \"" << arg << "\"";
      out_path = args [arg_index];
    }
    else if (arg == "-c" || arg == "--compress")
    {
      options.compress = true;
    }
    else if (arg == "-nc" || arg == "--no-compress")
    {
      options.compress = false;
    }
    else if (arg == "-m" || arg == "--mipmap")
    {
      options.mipmap = true;
    }
    else if (arg == "-nm" || arg == "--no-mipmap")
    {
      options.mipmap = false;
    }
    else if (arg == "-v" || arg == "--verbose")
    {
      options.verbose = true;
    }
    else if (arg == "-q" || arg == "--quiet")
    {
      options.verbose = false;
    }
    else if (arg == "-e" || arg == "--envmap")
    {
      if (in_path)
        throw Rk::Exception () << "Cannot specify a source-file with " << arg;
      cubemap = true;
    }
    else if (arg == "--front")
    {
      cubemap = true;
      if (face_paths [texface_front])
        throw Rk::Exception ("Only one front-face file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected front-face file path after \"" << arg << "\"";
      face_paths [texface_front] = args [arg_index];
    }
    else if (arg == "--back")
    {
      cubemap = true;
      if (face_paths [texface_back])
        throw Rk::Exception ("Only one back-face file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected back-face file path after \"" << arg << "\"";
      face_paths [texface_back] = args [arg_index];
    }
    else if (arg == "--left")
    {
      cubemap = true;
      if (face_paths [texface_left])
        throw Rk::Exception ("Only one left-face file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected left-face file path after \"" << arg << "\"";
      face_paths [texface_left] = args [arg_index];
    }
    else if (arg == "--right")
    {
      cubemap = true;
      if (face_paths [texface_right])
        throw Rk::Exception ("Only one right-face file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected right-face file path after \"" << arg << "\"";
      face_paths [texface_right] = args [arg_index];
    }
    else if (arg == "--top")
    {
      cubemap = true;
      if (face_paths [texface_top])
        throw Rk::Exception ("Only one top-face file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected top-face file path after \"" << arg << "\"";
      face_paths [texface_top] = args [arg_index];
    }
    else if (arg == "--bottom")
    {
      cubemap = true;
      if (face_paths [texface_bottom])
        throw Rk::Exception ("Only one bottom-face file may be specified");
      if (++arg_index == arg_count)
        throw Rk::Exception () << "Expected bottom-face file path after \"" << arg << "\"";
      face_paths [texface_bottom] = args [arg_index];
    }
    else if (arg == "-h" || arg == "--help" || arg == "/?")
    {
      Rk::std_err << "\nCo-Texture-Tool (c) 2011 Rk\n"
                  << "\n"
                  << "  Converts an image file to a Cobalt texture, optionally generating mipmaps\n"
                  << "  and compressing it in the process.\n"
                  << "\n"
                  << "Usage:\n"
                  << "  " << args [0] << " [-i] source-path [-o output-path] [-c] [-m] [-v]\n"
                  << "    Converts the image file at source-path to a Cobalt texture.\n"
                  << "\n"
                  << "  " << args [0] << " {-h|--help|/?}\n"
                  << "    Prints this message\n"
                  << "\n"
                  << "  " << args [0] << " [-e|--envmap] --top top-path --left left-path (etc)\n"
                  << "    Produces an environment (cube) map, using the specified face images.\n"
                  << "\n"
                  << "Options:\n"
                  << "  -i source-path      Uses source-path as the path to the image file to be\n"
                  << "                      converted.\n"
                  << "  -o output-path      Uses output-path as the path to the texture file to\n"
                  << "                      be created. If this file exists, it is overwritten\n"
                  << "                      without confirmation. If this option is not specified,\n"
                  << "                      then a file with a name similar to the source file in\n"
                  << "                      the current directory is used.\n"
                  << "  -e, --envmap        Produces an environment map. In this mode, paths to\n"
                  << "    --front path      source images for each of the cube faces are specified\n"
                  << "    --back path       by the corresponding parameters.\n"
                  << "    --left path       \n"
                  << "    --right path      \n"
                  << "    --top path        \n"
                  << "    --bottom path     \n"
                  << "  -c, --compress      Compresses the texture with DXT. Default.\n"
                  << "  -nc, --no-compress  Does not compress the texture.\n"
                  << "  -m, --mipmap        Generates mipmaps using a box filter. Default.\n"
                  << "  -nm, --no-mipamp    Does not generate mipmaps.\n"
                  << "  -v, --verbose       Verbose mode. Prints conversion information to the\n"
                  << "                      standard error stream.\n"
                  << "  -q, --quiet         Quiet mode. Prints nothing. Default.\n"
                  << "\n";
      return 0;
    }
    else
    {
      if (arg == "-i")
      {
        if (cubemap)
          throw Rk::Exception () << arg << " may not be used when producing an environment map";

        if (++arg_index == arg_count)
          throw Rk::Exception ("Expected source file path after \"-i\"");
      }
      else if (arg [0] == '-')
      {
        throw Rk::Exception () << "Unrecognized option \"" << arg << "\"";
      }
      
      if (cubemap)
          throw Rk::Exception () << "Unrecognized text \"" << arg << "\"";

      if (in_path)
        throw Rk::Exception ("Only one source file may be specified");

      in_path = args [arg_index];
    }
    
  }

  if (cubemap)
  {
    for (uint i = 0; i != 6; i++)
    {
      if (!face_paths [i])
        throw Rk::Exception ("Not all source files specified");
    }
  }
  else if (!in_path)
  {
    throw Rk::Exception ("No source file specified");
  }

  if (!out_path)
  {
    if (cubemap)
      throw Rk::Exception ("No output file specified");

    uptr dot = in_path.reverse_find ('.');
    if (dot == in_path.nowhere)
      dot = in_path.length ();
    out_path = in_path.range (0, dot);
    out_path += ".cotexture";
  }

  if (options.verbose)
  {
    if (cubemap)
      Rk::std_err << "Generating environment map " << out_path << '\n';
    else
      Rk::std_err << "Converting " << in_path << " to " << out_path << '\n';
  }

  Rk::File out_file (out_path, Rk::File::open_replace_or_create);
  out_file.delete_on_close (true);

  if (cubemap)
  {
    Rk::ImageFile in_files [6];
    for (uint i = 0; i != 6; i++)
      in_files [i].load (face_paths [i]);
    convert_cubemap (in_files, out_file);
  }
  else
  {
    Rk::ImageFile in_file (in_path);
    convert_texture (in_file, out_file);
  }

  return 0;
}
catch (const std::exception& e)
{
  Rk::std_err << "X " << e.what () << '\n';
  return 1;
}
catch (...)
{
  Rk::std_err << "X Exception\n";
  return 1;
}
