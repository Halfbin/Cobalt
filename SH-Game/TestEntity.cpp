//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxEntity.hpp>

// Uses
#include <Co/IxTextureFactory.hpp>
#include <Co/IxModelFactory.hpp>
#include <Co/IxLoadContext.hpp>
#include <Co/EntityClass.hpp>
#include <Co/IxTexture.hpp>
#include <Co/IxModel.hpp>
#include <Co/Spatial.hpp>

using namespace Co;

extern IxModelFactory*   model_factory;
extern IxTextureFactory* texture_factory;

namespace
{
  class TestEntity :
    public IxEntity
  {
    static IxModel::Ptr   model;
    static IxTexture::Ptr tex;
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

      spatial = next;
    }

  public:
    TestEntity (IxLoadContext& loadcontext, IxPropMap* props)
    {
      spatial.position.x = 0.0f;

      if (!model)
      {
        model = model_factory   -> create (loadcontext, "cube.rkmodel"  );
        tex   = texture_factory -> create (loadcontext, "derp.cotexture");
      }
    }

  }; // class TestEntity

  IxModel::Ptr   TestEntity::model;
  IxTexture::Ptr TestEntity::tex;
  Material       TestEntity::mat;

  EntityClass <TestEntity> ent_class ("TestEntity");

} // namespace

IxEntityClass* test_class = &ent_class;
