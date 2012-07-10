//
// Copyright (C) 2012 Roadkill Software
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

namespace SH
{
  class TestEntity :
    public Co::Entity
  {
    static Co::Model::Ptr   model;
    static Co::Texture::Ptr tex;
    static Co::Font::Ptr    font;
    static Co::Material     mat;

    Co::Spatial spatial;

    virtual void tick (float time, float prev_time, Co::WorkQueue& queue, const Co::KeyState* keyboard, v2f mouse_delta)
    {
      /*Co::Spatial next = spatial;
      next.orientation = Co::Quat (time * 2.0f, v3f (0.0f, 0.0f, 1.0f));

      if (!mat.diffuse_tex)
        mat.diffuse_tex = tex -> get ();
      model -> draw (frame, spatial, next, &mat, 1);*/

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

      //spatial = next;
    }

    virtual void render (Co::Frame& frame, float alpha)
    { }

    TestEntity (Co::WorkQueue& queue)
    {
      spatial.position = v3f (75.0f, 66.0f, 75.0f);
      spatial.orientation = nil;

      if (!model)
      {
        model = model_factory   -> create (queue, "cube.rkmodel");
        tex   = texture_factory -> create (queue, "derp.cotexture", false, true, true);

        Co::CodeRange ranges [] = {
          { 0x0020, 0x007f }, // ASCII
          { 0x00a1, 0x0100 }  // Latin-1 Supplement
        };
        font = font_factory -> create (queue, "../SH/Fonts/DejaVuSans.ttf", 14, Co::fontsize_points, ranges);
      }
    }

  public:
    static Ptr create (Co::WorkQueue& queue, const Co::PropMap* props)
    {
      return queue.gc_attach (new TestEntity (queue));
    }

  }; // class TestEntity

  Co::Model::Ptr   TestEntity::model;
  Co::Texture::Ptr TestEntity::tex;
  Co::Material     TestEntity::mat;
  Co::Font::Ptr    TestEntity::font;

  Co::EntityClass <TestEntity> ent_class ("TestEntity");
  Co::EntityClassBase& test_entity_class = ent_class;

} // namespace SH
