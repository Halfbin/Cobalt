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
  extern const Co::IxEntityClass
    *test_class,
    *block_world_class,
    *spectator_class;

  static const Co::IxEntityClass* const classes [] = {
    test_class,
    block_world_class,
    spectator_class
  };

  Co::IxModelFactory*   model_factory;
  Co::IxTextureFactory* texture_factory;
  Co::IxFontFactory*    font_factory;

  Rk::VirtualLockedOutStream log;

  class Game :
    public Co::IxGame
  {
    Co::IxEngine* engine;

    Co::IxModule::Ptr model_module;
    Co::IxModule::Ptr texture_module;
    Co::IxModule::Ptr font_module;

    virtual bool init    (Co::IxEngine* engine, Co::IxLoadContext* load_context, Rk::IxLockedOutStreamImpl* log_impl);
    virtual void destroy ();

    virtual void start ();
    virtual void stop  ();
    
    virtual void tick      (Co::IxFrame* frame, float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  public:
    void expose (void** out, u64 ixid);

  } game;

  bool Game::init (Co::IxEngine* new_engine, Co::IxLoadContext* load_context, Rk::IxLockedOutStreamImpl* log_impl) try
  {
    if (!new_engine || !log_impl)
      throw Rk::Exception ("null pointer");

    engine = new_engine;
    log.set_impl (log_impl);

    model_module = engine -> load_module ("Co-Model");
    model_module -> expose (model_factory);

    texture_module = engine -> load_module ("Co-Texture");
    texture_module -> expose (texture_factory);

    font_module = engine -> load_module ("Co-Font");
    font_module -> expose (font_factory);
    if (!font_factory -> init (log_impl))
      throw Rk::Exception ("error initializing font factory");

    engine -> register_classes (classes);

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
    texture_module.reset ();
    font_module.reset ();
    engine -> unregister_classes (classes);
  }

  void Game::start ()
  {
    engine -> create_entity ("TestEntity", 0);
    engine -> create_entity ("BlockWorld", 0);
    engine -> create_entity ("Spectator", 0);
  }

  void Game::stop ()
  {
    //ent -> destroy ();
  }

  void Game::tick (Co::IxFrame* frame, float time, float prev_time)
  {

  }

  void Game::update_ui (Co::IxUI* ui)
  {

  }

  void Game::expose (void** out, u64 ixid)
  {
    Rk::expose <Co::IxGame> (&game, ixid, out);
  }

  IX_EXPOSE (void** out, u64 ixid)
  {
    game.expose (out, ixid);
  }

} // namespace SH
