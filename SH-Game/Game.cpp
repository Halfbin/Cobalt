//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxGame.hpp>

// Uses
#include <Co/IxTextureFactory.hpp>
#include <Co/IxModelFactory.hpp>
#include <Co/IxFontFactory.hpp>
#include <Co/IxEngine.hpp>
//#include <Co/IxEntity.hpp>
#include <Co/IxModule.hpp>

#include <Rk/Expose.hpp>

Co::IxModelFactory*   model_factory;
Co::IxTextureFactory* texture_factory;
Co::IxFontFactory*    font_factory;

namespace
{
  class Game :
    public Co::IxGame
  {
    Co::IxModule* model_module;
    Co::IxModule* texture_module;
    Co::IxModule* font_module;

    virtual void init (Co::IxEngine& engine, Rk::IxLockedOutStreamImpl* log_impl);
    
    virtual void start (Co::IxEngine& engine);
    virtual void stop  ();
    
    virtual void tick      (float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  public:
    void expose (void** out, u64 ixid);

  } game;

  void Game::init (Co::IxEngine& engine, Rk::IxLockedOutStreamImpl* log_impl)
  {
    model_module = engine.load_module ("Co-Model");
    model_module -> expose (model_factory);

    texture_module = engine.load_module ("Co-Texture");
    texture_module -> expose (texture_factory);

    font_module = engine.load_module ("Co-Font");
    font_module -> expose (font_factory);
    font_factory -> init (log_impl);
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

  void Game::expose (void** out, u64 ixid)
  {
    Rk::expose <Co::IxGame> (&game, ixid, out);
  }

} // namespace

void expose_game (u64 ixid, void** out)
{
  return game.expose (out, ixid);
}
