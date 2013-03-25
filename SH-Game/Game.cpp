//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Game.hpp>

// Uses
#include <Co/Texture.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>

#include <Rk/Exception.hpp>
#include <Rk/Module.hpp>

#include "Common.hpp"
#include "World.hpp"
#include "Pawn.hpp"

namespace SH
{
  class Game :
    public Co::Game
  {
    World::Ptr world;
    Pawn::Ptr  pawn;
    
    virtual void client_start ();
    virtual void client_stop  ();
    virtual void server_start ();
    virtual void server_stop  ();

    virtual void client_tick (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count, const Co::KeyState* kb, v2f mouse_delta);
    virtual void server_tick (float time, float step, Co::WorkQueue& queue);

    virtual void render (Co::Frame& frame, float alpha);

  };

  RK_MODULE_FACTORY (Game);

  void Game::init (Co::WorkQueue& queue, Co::RenderContext& rc, Co::Log& new_log)
  {
    log_ptr = &new_log;
    log () << "- SH-Game: Initializing\n";

    engine.get_object (texture_factory);
    engine.get_object (model_factory);
    engine.get_object (font_factory);

    world = World::create (0xdecafbadfeedbeefull, queue, rc);
    pawn = create_spectator (nil);
  }

  void Game::start (Co::Engine& engine)
  {
    log () << "- SH-Game: Starting\n";

    engine.enable_ui (false);
  }

  void Game::stop ()
  {
    log () << "- SH-Game: Stopping\n";
  }

  void Game::tick (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count, const Co::KeyState* kb, v2f mouse_delta)
  {
    pawn -> tick (time, step, kb, mouse_delta);

    Co::Spatial cur, next;
    pawn -> get_view (cur, next);

    world -> tick (time, step, queue, cur);
  }

  void Game::render (Co::Frame& frame, float alpha)
  {
    pawn -> render (frame, alpha);

    Co::Spatial cur, next;
    pawn -> get_view (cur, next);

    world -> render (frame, lerp (cur, next, alpha));
  }

} // namespace SH
