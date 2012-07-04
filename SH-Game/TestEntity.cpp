//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Entity.hpp>

// Uses
#include <Co/EntityClass.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/Spatial.hpp>
#include <Co/Texture.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>

#include <Rk/ShortString.hpp>

#include <array>

#include "Common.hpp"

using namespace Co;

namespace SH
{
  void draw_ui_text (Frame& frame, int x, int y, Font* font, Rk::U16StringRef text)
  {
    if (!font -> ready ())
      return;

    Rk::ShortU32String <1024> buffer;
    for (auto iter = text.begin (); iter != text.end (); iter++)
      buffer.append (char32 (*iter));

    Character chars [1024];
    font -> translate_codepoints (buffer.data (), text.length (), chars);

    TexRect rects [1024];

    for (uint i = 0; i != text.length (); i++)
    {
      const auto& character = chars [i];
      const auto& metrics   = font -> metrics (character.index);
      auto&       rect      = rects [i];

      x += character.kerning;
      rect.x = x + metrics.bearing_x;
      rect.y = y - metrics.bearing_y;
      rect.w = metrics.width;
      rect.h = metrics.height;
      rect.s = metrics.x;
      rect.t = metrics.y;
      rect.tw = metrics.width;
      rect.th = metrics.height;
      x += metrics.advance;
    }

    //frame.add_label (font -> image (), rects, text.length (), Co::Spatial2D (), Co::Spatial2D ());
  }

  class TestEntity :
    public Entity
  {
    static Model::Ptr   model;
    static Texture::Ptr tex;
    static Font::Ptr    font;
    static Co::Material mat;

    Spatial spatial;

    virtual void tick (Frame& frame, float time, float prev_time, const KeyState* keyboard, v2f mouse_delta)
    {
      Spatial next = spatial;
      next.orientation = Quaternion (time * 2.0f, Vector3 (0.0f, 0.0f, 1.0f));

      if (!mat.diffuse_tex)
        mat.diffuse_tex = tex -> get ();
      model -> draw (frame, spatial, next, &mat, 1);

      /*draw_ui_text (frame, 100, 100, font, Rk::u16_string (L"Hello world"));
      draw_ui_text (frame, 100, 200, font, Rk::u16_string (L"This is a test!"));
      draw_ui_text (frame, 100, 300, font,
        Rk::u16_string (L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
      );
      draw_ui_text (frame, 100, 400, font,
        Rk::u16_string (L"0123456789!\"£$%^&*()-=_+[]{};'#:@~,./<>?\\|`¬¦")
      );*/

      /*draw_ui_text (frame, 200, 600, font,
        Rk::u16_string (L"Testing Testing 123")
      );*/

      spatial = next;
    }

    TestEntity (WorkQueue& queue)
    {
      spatial.position = Vector3 (75.0f, 66.0f, 75.0f);
      spatial.orientation = nil;

      if (!model)
      {
        model = model_factory   -> create (queue, "cube.rkmodel");
        tex   = texture_factory -> create (queue, "derp.cotexture", false, true);

        CodeRange ranges [] = {
          { 0x0020, 0x007f }, // ASCII
          { 0x00a1, 0x0100 }  // Latin-1 Supplement
        };
        font = font_factory -> create (queue, "../SH/Fonts/DejaVuSans.ttf", 14, fontsize_points, ranges);
      }
    }

  public:
    static Ptr create (Co::WorkQueue& queue, const Co::PropMap* props)
    {
      return queue.gc_attach (new TestEntity (queue));
    }

  }; // class TestEntity

  Model::Ptr   TestEntity::model;
  Texture::Ptr TestEntity::tex;
  Material     TestEntity::mat;
  Font::Ptr    TestEntity::font;

  EntityClass <TestEntity> ent_class ("TestEntity");
  Co::EntityClassBase& test_entity_class = ent_class;

} // namespace SH
