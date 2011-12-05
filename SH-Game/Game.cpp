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

#include <Rk/Exception.hpp>
#include <Rk/Expose.hpp>

namespace SH
{
  Co::IxModelFactory*   model_factory;
  Co::IxTextureFactory* texture_factory;
  Co::IxFontFactory*    font_factory;

  Rk::VirtualLockedOutStream log;

  class Game :
    public Co::IxGame
  {
    Co::IxModule::Ptr model_module;
    Co::IxModule::Ptr texture_module;
    Co::IxModule::Ptr font_module;

    virtual bool init    (Co::IxEngine* engine, Rk::IxLockedOutStreamImpl* log_impl);
    virtual void destroy ();

    virtual void start (Co::IxEngine* engine);
    virtual void stop  ();
    
    virtual void tick      (float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  public:
    void expose (void** out, u64 ixid);

  } game;

  bool Game::init (Co::IxEngine* engine, Rk::IxLockedOutStreamImpl* log_impl) try
  {
    if (!engine || !log_impl)
      throw Rk::Exception ("null pointer");

    log.set_impl (log_impl);

    model_module = engine -> load_module ("Co-Model");
    model_module -> expose (model_factory);

    texture_module = engine -> load_module ("Co-Texture");
    texture_module -> expose (texture_factory);

    font_module = engine -> load_module ("Co-Font");
    font_module -> expose (font_factory);
    if (!font_factory -> init (log_impl))
      throw Rk::Exception ("error initializing font factory");

    return true;
  }
  catch (const std::exception& e)
  {
    if (log)
      log << "X " << e.what () << '\n';
    return false;
  }
  catch (...)
  {
    if (log)
      log << "X Exception caught\n";
    return false;
  }

  void Game::destroy ()
  {
    model_module.reset ();
    model_module.reset ();
    model_module.reset ();
  }

  void Game::start (Co::IxEngine* engine)
  {
    engine -> create_entity ("TestEntity", 0);
    engine -> create_entity ("BlockWorld", 0);
    engine -> create_entity ("Spectator", 0);
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

  void expose_game (u64 ixid, void** out)
  {
    return game.expose (out, ixid);
  }

} // namespace SH
