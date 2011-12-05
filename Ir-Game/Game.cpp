//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxGame.hpp>

// Uses
#include <Co/IxEngine.hpp>

#include <Rk/Expose.hpp>

#include "State.hpp"

namespace Ir
{
  /*static const Co::IxEntityClass* const classes [] = {
    
  };*/

  class Game :
    public Co::IxGame
  {
    Co::IxEngine* engine;

    State *state,
          *next_state;

    virtual bool init    (Co::IxEngine* engine, Rk::IxLockedOutStreamImpl* log_impl);
    virtual void destroy ();

    virtual void start ();
    virtual void stop  ();
    
    virtual void tick      (float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  public:
    void expose (void** out, u64 ixid);

  } game;

  bool Game::init (Co::IxEngine* new_engine, Rk::IxLockedOutStreamImpl* log_impl) try
  {
    if (!new_engine || !log_impl)
      return false;

    engine = new_engine;
    //engine -> register_classes (classes);

    return true;
  }
  catch (...)
  {
    return false;
  }

  void Game::destroy ()
  {
    //engine -> unregister_classes (classes);
  }

  void Game::start ()
  {
    state      = 0;
    next_state = title_state;
  }

  void Game::stop ()
  {
    if (state)
      state -> leave ();
  }

  void Game::tick (float time, float prev_time)
  {
    if (next_state)
    {
      if (state)
        state -> leave ();
      state = next_state;
      if (state)
        state -> enter ();
      next_state = 0;
    }

    state -> tick (time, prev_time);
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
