//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Co/TextureFile.hpp>

#include <Rk/ShortString.hpp>
#include <Rk/Exception.hpp>
#include <Rk/ImageFile.hpp>
#include <Rk/DXTCodec.hpp>
#include <Rk/Console.hpp>
#include <Rk/Vector4.hpp>
#include <Rk/File.hpp>

#include <vector>

namespace Co
{
  struct Options
  {
    bool compress,
         mipmap,
         verbose,
         emit_alpha,
         same_alpha,
         tile_array;
    uint tile_rows,
         tile_columns;

    Options () :
      compress   (true),
      mipmap     (true),
      verbose    (false),
      same_alpha (true),
      tile_array (false)
    { }
  
  } options;

  //
  // = Box Filter ========================================================================================================
  //
  void box_minify_integral (Rk::Image& dest, const Rk::Image& src)
  {
    uint x_stride = src.width  / dest.width,
         y_stride = src.height / dest.height;

    uint comp_count = 3;
    if (src.pixel_type == Rk::rgba_8888 && dest.pixel_type == Rk::rgba_8888)
      comp_count = 4;

    u8* dest_row = dest.data;
    u8* src_row  = src.data;

    for (auto dy = 0; dy != dest.height; dy++)
    {
      u8* dest_pixel = dest_row;
      u8* src_pixel  = src_row;

      for (auto dx = 0; dx != dest.width; dx++)
      {
        u64 result [4] = { 0, 0, 0, 0 };

        u8* block_row = src_pixel;

        for (auto by = 0; by != y_stride; by++)
        {
          u8* block_pixel = block_row;

          for (auto bx = 0; bx != x_stride; bx++)
          {
            for (auto comp = 0; comp != comp_count; comp++)
              result [comp] += block_pixel [comp];

            block_pixel += src.pixel_stride;
          }

          block_row += src.row_stride;
        }

        auto block_area = x_stride * y_stride;

        for (auto comp = 0; comp != comp_count; comp++)
          dest_pixel [comp] = u8 (result [comp] / block_area);

        if (comp_count == 3 && dest.pixel_type == Rk::rgba_8888)
          dest_pixel [3] = 255;

        dest_pixel += dest.pixel_stride;
        src_pixel  += src.pixel_stride * x_stride;
      }

      dest_row += dest.row_stride;
      src_row  += src.row_stride * y_stride;
    }
  }

  //
  // = Mipmap generator - DXT ============================================================================================
  //
  static u8* build_dxt_mipmap (
    u8*              dest,
    const Rk::Image& src,
    uint             level)
  {
    const uint factor = 1 << level;
    const uint block_size = factor * 4;

    u8* src_row = src.data;

    for (uint sy = 0; sy != src.height; sy += block_size)
    {
      u8* src_px = src_row;

      for (uint sx = 0; sx != src.width; sx += block_size)
      {
        Rk::Image src_block;
        src_block.reset (src_px, block_size, block_size, src.pixel_type, src.row_stride, src.pixel_stride);

        u8 buffer [64];
        Rk::Image min_block;
        min_block.reset (buffer, 4, 4, Rk::rgba_8888);

        box_minify_integral (min_block, src_block);

        dest = Rk::dxt_encode_block (dest, min_block.data, options.emit_alpha);

        src_px += block_size * src.pixel_stride;
      }

      src_row += block_size * src.row_stride;
    }

    return dest;
  }

  //
  // = COTEXTR1 writer ===================================================================================================
  //
  static void write_texture (Rk::File& out_file, u8 mipmap_count, u8 layer_count, u8 flags, TexFormat format, u16 width, u16 height, const u8* data, u32 data_size)
  {
    if (options.verbose)
      Rk::std_err << "Output format is " << tex_format_strings [format] << '\n';

    TextureHeader header = {
      0x20120711,
      flags,
      mipmap_count,
      layer_count,
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

  static const char* const format_strings [8] = {
    "<?>"
    "rgb565",
    "bgr565",
    "rgb888",
    "bgr888",
    "rgba8888",
    "bgra8888",
    "i8"
  };

  static void check_image (const Rk::Image& image)
  {
    if (image.width > 65536 || image.height > 65536)
      throw std::runtime_error ("Source images may not be wider or taller than 65536 pixels");

    if (options.compress && (image.width % 4 || image.height % 4))
      throw std::runtime_error ("Dimensions of source images must be multiples of 4 when compression is on");
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
      mipmap_size = width * height * (options.emit_alpha ? 4 : 3);
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

  static u8* build_raw_mipmaps (u8* dest, const Rk::Image& src, u8 mipmap_count)
  {
    Rk::Image dest_map;
    dest_map.reset (dest, src.width, src.height, options.emit_alpha ? Rk::rgba_8888 : Rk::rgb_888);
  
    for (uint level = 0; level != mipmap_count; level++)
    {
      box_minify_integral (dest_map, src);
      dest_map.reset (dest_map.data + dest_map.size (), dest_map.width / 2, dest_map.height / 2, dest_map.pixel_type);
    }

    return dest_map.data;
  }

  static u8* build_mipmaps (u8* dest, const Rk::Image& src, u8 mipmap_count)
  {
    if (options.compress)
    {
      for (uint level = 0; level != mipmap_count; level++)
        dest = build_dxt_mipmap (dest, src, level);
    }
    else
    {
      dest = build_raw_mipmaps (dest, src, mipmap_count);
    }

    return dest;
  }

  static void split_array (u8* dest, const Rk::Image& src, u8 mipmap_count, uint& tile_width, uint& tile_height)
  {
    tile_width  = src.width  / options.tile_columns;
    tile_height = src.height / options.tile_rows;

    if (src.width % options.tile_columns || src.height % options.tile_rows)
      throw std::runtime_error ("Source image dimensions must be divisible by tile grid dimensions");

    u8* tile_row = src.data;

    for (uint tile_y = 0; tile_y != src.height; tile_y += tile_height)
    {
      u8* tile_column = tile_row;

      for (uint tile_x = 0; tile_x != src.width; tile_x += tile_width)
      {
        Rk::Image tile;
        tile.reset (tile_column, tile_width, tile_height, src.pixel_type, src.row_stride, src.pixel_stride);

        dest = build_mipmaps (dest, tile, mipmap_count);

        tile_column += tile_width * src.pixel_stride;
      }

      tile_row += tile_height * src.row_stride;
    }
  }

  static void convert_texture (Rk::File& dest, const Rk::Image& src)
  {
    if (options.verbose)
      Rk::std_err << "Source image is " << src.width << 'x' << src.height << ":" << format_strings [src.pixel_type] << '\n';

    check_image (src);
  
    u8  mipmap_count;
    u32 data_size;
    compute_output_info (src.width, src.height, mipmap_count, data_size);

    std::vector <u8> buffer (data_size);
    uint width  = src.width,
         height = src.height;

    if (options.tile_array)
      split_array (buffer.data (), src, mipmap_count, width, height);
    else
      build_mipmaps (buffer.data (), src, mipmap_count);

    Co::TexFormat tex_format;
    if (options.compress)
      tex_format = options.emit_alpha ? texformat_dxt3 : texformat_dxt1;
    else
      tex_format = options.emit_alpha ? texformat_rgba8888 : texformat_rgb888;

    write_texture (
      dest,
      mipmap_count,
      options.tile_columns * options.tile_rows,
      0,
      tex_format,
      width  & 0xffff,
      height & 0xffff,
      buffer.data (),
      (u32) buffer.size ()
    );

    dest.delete_on_close (false);
  }

  void convert_cubemap (Rk::Image (&in_images) [6], Rk::File& out_file)
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
        throw std::runtime_error ("Source images differ in format or dimensions");
      }
    }

    u8  mipmap_count;
    u32 data_size;
    compute_output_info (width, height, mipmap_count, data_size);

    std::unique_ptr <u8 []> buffer (new u8 [data_size * 6]);
    u8* ptr = buffer.get ();
    for (uint i = 0; i != 6; i++)
      ptr = build_mipmaps (ptr, in_images [i], mipmap_count);

    write_texture (
      out_file,
      mipmap_count,
      0,
      texflag_cube_map,
      options.compress ? texformat_dxt1 : texformat_rgba8888,
      width  & 0xffff,
      height & 0xffff,
      buffer.get (),
      data_size * 6
    );

    out_file.delete_on_close (false);
  }

  void load_convert (Rk::Image& image, Rk::StringRef path)
  {
    Rk::load (image, path);

    if (image.pixel_type == Rk::bgr_888 || image.pixel_type == Rk::bgra_8888)
    {
      u8* row = image.data;
      for (uint y = 0; y != image.height; y++)
      {
        u8* px = row;
        for (uint x = 0; x != image.width; x++)
        {
          std::swap (px [0], px [2]);
          px += image.pixel_stride;
        }
        row += image.row_stride;
      }

      if (image.pixel_type == Rk::bgr_888)
        image.pixel_type = Rk::rgb_888;
      else
        image.pixel_type = Rk::rgba_8888;
    }

  }

  int main (int arg_count, const char** args) try
  {
    Rk::StringRef         in_path,
                          face_paths [6];
    Rk::ShortString <512> out_path;
    bool                  cubemap = false;

    for (int arg_index = 1; arg_index != arg_count; arg_index++)
    {
      Rk::StringRef arg = args [arg_index];

      if (arg == "-o")
      {
        if (out_path)
          throw std::runtime_error ("Only one destination file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected destination file path after " << arg;
        out_path = args [arg_index];
      }
      else if (arg == "-t" || arg == "--tile")
      {
        if (cubemap)
          Rk::raise () << "Cannot use " << arg << " when producing an environment map";
        if (++arg_index + 1 >= arg_count)
          Rk::raise () << "Expected columns and rows after " << arg;
        options.tile_array   = true;
        options.tile_columns = std::atoi (args [arg_index++]);
        options.tile_rows    = std::atoi (args [arg_index++]);
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
      else if (arg == "-a" || arg == "--alpha")
      {
        options.emit_alpha = true;
        options.same_alpha = false;
      }
      else if (arg == "-na" || arg == "--no-alpha")
      {
        options.emit_alpha = false;
        options.same_alpha = false;
      }
      else if (arg == "-sa" || arg == "--same-alpha")
      {
        options.same_alpha = true;
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
          Rk::raise () << "Cannot specify a source-file with " << arg;
        cubemap = true;
      }
      else if (arg == "--front")
      {
        cubemap = true;
        if (face_paths [texface_front])
          throw std::runtime_error ("Only one front-face file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected front-face file path after " << arg;
        face_paths [texface_front] = args [arg_index];
      }
      else if (arg == "--back")
      {
        cubemap = true;
        if (face_paths [texface_back])
          throw std::runtime_error ("Only one back-face file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected back-face file path after " << arg;
        face_paths [texface_back] = args [arg_index];
      }
      else if (arg == "--left")
      {
        cubemap = true;
        if (face_paths [texface_left])
          throw std::runtime_error ("Only one left-face file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected left-face file path after " << arg;
        face_paths [texface_left] = args [arg_index];
      }
      else if (arg == "--right")
      {
        cubemap = true;
        if (face_paths [texface_right])
          throw std::runtime_error ("Only one right-face file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected right-face file path after " << arg;
        face_paths [texface_right] = args [arg_index];
      }
      else if (arg == "--top")
      {
        cubemap = true;
        if (face_paths [texface_top])
          throw std::runtime_error ("Only one top-face file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected top-face file path after " << arg;
        face_paths [texface_top] = args [arg_index];
      }
      else if (arg == "--bottom")
      {
        cubemap = true;
        if (face_paths [texface_bottom])
          throw std::runtime_error ("Only one bottom-face file may be specified");
        if (++arg_index == arg_count)
          Rk::raise () << "Expected bottom-face file path after " << arg;
        face_paths [texface_bottom] = args [arg_index];
      }
      else if (arg == "-h" || arg == "--help" || arg == "/?")
      {
        Rk::std_err 
          << "\nCo-Texture-Tool (c) 2012 Rk\n"
          << "\n"
          << "  Converts an image file to a Cobalt texture, optionally generating mipmaps\n"
          << "  and compressing it in the process.\n"
          << "\n"
          << "Usage:\n"
          << "  Co-Texture-Tool [-i] source-path [-o output-path] [options]\n"
          << "    Converts the source image to a Cobalt texture.\n"
          << "\n"
          << "  Co-Texture-Tool {-t|--tile} columns rows [-i] source-path [-o output-path]\n"
          << "    Produces an array texture by slicing the source image into a regular\n"
          << "    rectangular grid of tiles, and using each tile as a layer in the array,\n"
          << "    proceeding from the top-left tile, row by row.\n"
          << "\n"
          << "  Co-Texture-Tool [-e|--envmap] --top top-path --left left-path (etc)\n"
          << "    Produces an environment (cube) map, using the specified face images.\n"
          << "\n"
          << "  Co-Texture-Tool {-h|--help|/?}\n"
          << "    Prints this message.\n"
          << "\n"
          << "Single texture options:\n"
          << "  -i source-path      Uses source-path as the path to the image file to be\n"
          << "                      converted.\n"
          << "  -o output-path      Uses output-path as the path to the texture file to\n"
          << "                      be created. If this file exists, it is overwritten\n"
          << "                      without confirmation. If this option is not specified,\n"
          << "                      then a file with a name similar to the source file in\n"
          << "                      the current directory is used.\n"
          << "\n"
          << "Tiled array options:\n"
          << "  -t cols rows        Specifies the number of rows and columns in the tile\n"
          << "  --tile cols rows    grid used to slice the source image into layers. The\n"
          << "                      width and height of the source image must be divisible\n"
          << "                      by cols and rows respectively.\n"
          << "\n"
          << "Environment map options:\n"
          << "  -e, --envmap        Produces an environment map. In this mode, paths to\n"
          << "    --front  path     source images for each of the cube faces are specified\n"
          << "    --back   path     by the corresponding parameters.\n"
          << "    --left   path     \n"
          << "    --right  path     \n"
          << "    --top    path     \n"
          << "    --bottom path     \n"
          << "\n"
          << "Common options:\n"
          << "  -c, --compress      Compresses the texture with DXT. Default.\n"
          << "  -nc, --no-compress  Does not compress the texture.\n"
          << "  -m, --mipmap        Generates mipmaps using a box filter. Default.\n"
          << "  -nm, --no-mipamp    Does not generate mipmaps.\n"
          << "  -a, --alpha         Emits alpha channel.\n"
          << "  -na, --no-alpha     Does not emit alpha channel.\n"
          << "  -sa, --same-alpha   Transfers alpha channel from source. Default.\n"
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
            throw std::runtime_error ("-i may not be used when producing an environment map");

          if (++arg_index == arg_count)
            throw std::runtime_error ("Expected source file path after -i");
        }
        else if (arg [0] == '-')
        {
          Rk::raise () << "Unrecognized option " << arg;
        }
      
        if (cubemap)
          Rk::raise () << "Unrecognized text " << arg;

        if (in_path)
          throw std::runtime_error ("Only one source file may be specified");

        in_path = args [arg_index];
      }
    
    }

    if (cubemap)
    {
      if (options.tile_array)
        throw std::runtime_error ("Inconsistent options - Cannot produce tiled array environment map");

      for (uint i = 0; i != 6; i++)
      {
        if (!face_paths [i])
          throw std::runtime_error ("Not all source files specified");
      }
    }
    else if (!in_path)
    {
      throw std::runtime_error ("No source file specified");
    }

    if (!out_path)
    {
      if (cubemap)
        throw std::runtime_error ("No output file specified");

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
      Rk::Image in_files [6];
      for (uint i = 0; i != 6; i++)
        load_convert (in_files [i], face_paths [i]);

      convert_cubemap (in_files, out_file);
    }
    else
    {
      Rk::Image in_file;
      load_convert (in_file, in_path);

      if (options.same_alpha)
        options.emit_alpha = (in_file.pixel_type == Rk::rgba_8888);

      convert_texture (out_file, in_file);
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

}

int main (int arg_count, const char** args)
{
  return Co::main (arg_count, args);
}
