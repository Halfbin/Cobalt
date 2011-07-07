//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/IxGame.hpp>

namespace
{
  class Game :
    public Co::IxGame
  {
    virtual void init (Co::IxEngine* api);
    
    virtual void start ();
    virtual void stop  ();
    
    virtual void tick (float time, float prev_time);
    virtual void update_ui (Co::IxUI* ui);

  } game;

  void Game::init (Co::IxEngine* api)
  {

  }

  void Game::start ()
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

} // namespace

Co::IxGame* ix_game = &game;
