//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxEntity.hpp>

// Uses
#include <Co/IxTextureFactory.hpp>
#include <Co/IxModelFactory.hpp>
#include <Co/IxFontFactory.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/EntityClass.hpp>
#include <Co/IxTexture.hpp>
#include <Co/IxModel.hpp>
#include <Co/Spatial.hpp>
#include <Co/IxFont.hpp>

#include <Rk/ShortString.hpp>

#include <array>

using namespace Co;

extern IxModelFactory*   model_factory;
extern IxTextureFactory* texture_factory;
extern IxFontFactory*    font_factory;

namespace
{
  void draw_ui_text (Frame& frame, int x, int y, IxFont* font, Rk::U16StringRef text)
  {
    if (!font -> ready ())
      return;

    Rk::ShortU32String <1024> buffer;
    for (auto iter = text.begin (); iter != text.end (); iter++)
      buffer.append (char32 (*iter));

    u32 indices [1024];
    font -> translate_codepoints (buffer.data (), text.length (), indices);

    TexRect rects [1024];

    for (uint glyph = 0; glyph != text.length (); glyph++)
    {
      const GlyphMetrics& metrics = font -> get_metrics (indices [glyph]);
      TexRect rect;
      rect.x = x + metrics.bearing_x;
      rect.y = y + metrics.bearing_y;
      rect.w = metrics.width;
      rect.h = metrics.height;
      rect.s = metrics.x;
      rect.t = metrics.y;
      rect.tw = metrics.width;
      rect.th = metrics.height;
      rects [glyph] = rect;
      x += metrics.advance;
    }

    frame.add_ui_batch (font -> get_image (), rects, rects + text.length ());
  }

  class TestEntity :
    public IxEntity
  {
    static IxModel::Ptr   model;
    static IxTexture::Ptr tex;
    static IxFont::Ptr    font;
    static Co::Material   mat;

    Spatial spatial;

    virtual void destroy ()
    {
      delete this;
    }

    virtual void tick (float time, float prev_time, Frame& frame)
    {
      Spatial cam_a = spatial,
              cam_b = spatial;

      cam_a.orientation = Quaternion (prev_time, Vector3 (1, 0, 0));
      cam_a.position.x -= 8.5f + 6.0f * std::sin (prev_time * 2.0f);
      cam_b.orientation = Quaternion (time, Vector3 (1, 0, 0));
      cam_b.position.x -= 8.5f + 6.0f * std::sin (time * 2.0f);

      frame.set_camera (cam_a, cam_b, 75.0f, 75.0f, 0.1f, 1000.0f);

      Spatial next = spatial;
      next.orientation = Quaternion (time * 2.0f, Vector3 (0.0f, 0.0f, 1.0f));

      if (!mat.diffuse_tex)
        mat.diffuse_tex = tex -> get ();
      model -> draw (frame, spatial, next, &mat, 1);

      draw_ui_text (frame, 100, 100, font, Rk::u16_string (L"Hello world"));

      spatial = next;
    }

  public:
    TestEntity (IxLoadContext& loadcontext, IxPropMap* props)
    {
      spatial.position.x = 0.0f;

      if (!model)
      {
        model = model_factory   -> create (loadcontext, "cube.rkmodel");
        tex   = texture_factory -> create (loadcontext, "derp.cotexture");
        CodeRange codepoints = { 0x0020, 0x007f }; // ASCII printable
        font  = font_factory    -> create (loadcontext, "DejaVuSans.ttf", 32, fontsize_points, &codepoints, &codepoints + 1);
      }
    }

  }; // class TestEntity

  IxModel::Ptr   TestEntity::model;
  IxTexture::Ptr TestEntity::tex;
  Material       TestEntity::mat;
  IxFont::Ptr    TestEntity::font;

  EntityClass <TestEntity> ent_class ("TestEntity");

} // namespace

IxEntityClass* test_class = &ent_class;
