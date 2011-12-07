//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxGame.hpp>

// Uses
#include <Co/IxModule.hpp>
#include <Co/IxFrame.hpp>

#include <Rk/VirtualOutStream.hpp>
#include <Rk/Expose.hpp>

#include "Common.hpp"
#include "State.hpp"

namespace Ir
{
  /*static const Co::IxEntityClass* const classes [] = {
    
  };*/

  Co::IxTextureFactory* texture_factory;
  Co::IxModelFactory*   model_factory;
  Co::IxFontFactory*    font_factory;
  Co::IxLoadContext*    load_context;
  Co::IxEngine*         engine;

  Rk::VirtualLockedOutStream log;

  class Game :
    public Co::IxGame
  {
    Co::IxModule::Ptr texture_module,
                      model_module,
                      font_module;

    State *state,
          *next_state;

    virtual bool init    (Co::IxEngine* engine, Co::IxLoadContext* load_context, Rk::IxLockedOutStreamImpl* log_impl);
    virtual void destroy ();

    virtual void start ();
    virtual void stop  ();
    
    virtual void tick      (Co::IxFrame* frame, float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  public:
    void expose (void** out, u64 ixid);

  } game;

  bool Game::init (Co::IxEngine* new_engine, Co::IxLoadContext* new_load_context, Rk::IxLockedOutStreamImpl* log_impl) try
  {
    if (!new_engine || !new_load_context || !log_impl)
      return false;

    engine       = new_engine;
    load_context = new_load_context;
    log.set_impl (log_impl);

    log << "- Ir-Game: Initializing\n";

    texture_module = engine -> load_module ("Co-Texture");
    texture_module -> expose (texture_factory);

    model_module = engine -> load_module ("Co-Model");
    model_module -> expose (model_factory);
    
    font_module = engine -> load_module ("Co-Font");
    font_module -> expose (font_factory);

    //engine -> register_classes (classes);

    return true;
  }
  catch (...)
  {
    return false;
  }

  void Game::destroy ()
  {
    log << "- Ir-Game: Shutting down\n";

    //engine -> unregister_classes (classes);

    texture_module = nil;
    model_module   = nil;
    font_module    = nil;
  }

  void Game::start ()
  {
    log << "- Ir-Game: Starting\n";

    engine -> game_starting (this);

    state      = 0;
    next_state = title_state;
  }

  void Game::stop ()
  {
    log << "- Ir-Game: Stopping\n";

    engine -> game_stopping (this);

    if (state)
      state -> leave ();
  }

  void Game::tick (Co::IxFrame* frame, float time, float prev_time)
  {
    if (next_state)
    {
      if (state)
        state -> leave ();
      state = next_state;
      if (state)
        state -> enter (time);
      next_state = 0;
    }

    state -> tick (frame, time, prev_time);
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

} // namespace Ir
