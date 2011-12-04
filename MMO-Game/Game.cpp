//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxGame.hpp>

// Uses
#include <Co/IxEngine.hpp>

#include <Rk/Expose.hpp>

namespace
{
  class Game :
    public Co::IxGame
  {
    virtual void init (Co::IxEngine* engine, Rk::IxLockedOutStreamImpl* log_impl);
    
    virtual void start (Co::IxEngine* engine);
    virtual void stop  ();
    
    virtual void tick      (float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  public:
    void expose (void** out, u64 ixid);

  } game;

  void Game::init (Co::IxEngine* engine, Rk::IxLockedOutStreamImpl* log_impl)
  {
    
  }

  void Game::start (Co::IxEngine* engine)
  {
    
  }

  void Game::stop ()
  {
    
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

IX_EXPOSE (void** out, u64 ixid)
{
  game.expose (out, ixid);
}
