//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxGame.hpp>

// Uses
#include <Co/IxTextureFactory.hpp>
#include <Co/IxModelFactory.hpp>
#include <Co/IxEngine.hpp>
//#include <Co/IxEntity.hpp>
#include <Co/IxModule.hpp>

#include <Rk/Expose.hpp>

Co::IxModelFactory*   model_factory;
Co::IxTextureFactory* texture_factory;

namespace
{
  class Game :
    public Co::IxGame
  {
    Co::IxModule* model_module;
    Co::IxModule* texture_module;

    virtual void init (Co::IxEngine& engine);
    
    virtual void start (Co::IxEngine& engine);
    virtual void stop  ();
    
    virtual void tick      (float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  } game;

  void Game::init (Co::IxEngine& engine)
  {
    model_module = engine.load_module ("Co-Model");
    model_module -> expose (model_factory);

    texture_module = engine.load_module ("Co-Texture");
    texture_module -> expose (texture_factory);
  }

  void Game::start (Co::IxEngine& engine)
  {
    engine.create_entity ("TestEntity", 0);
  }

  void Game::stop ()
  {
    //ent -> destroy ();
  }

  void Game::tick (float time, float prev_time)
  {

  }

  void Game::update_ui (Co::IxUI* ui)
  {

  }

} // namespace

void expose_game (u64 ixid, void** out)
{
  Rk::expose <Co::IxGame> (&game, ixid, out);
}
