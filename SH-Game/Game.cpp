//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Game.hpp>

// Uses
#include <Co/Texture.hpp>
#include <Co/Engine.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>

#include <Rk/Exception.hpp>
#include <Rk/Module.hpp>

#include "Common.hpp"
#include "World.hpp"
#include "Pawn.hpp"

namespace SH
{
  Co::ModelFactory::Ptr   model_factory;
  Co::TextureFactory::Ptr texture_factory;
  Co::FontFactory::Ptr    font_factory;

  Co::Log* log_ptr = 0;

  class Game :
    public Co::Game
  {
    World::Ptr world;
    Pawn::Ptr  pawn;
    
    virtual void init (Co::Engine& engine, Co::WorkQueue& queue, Co::RenderContext& rc, Co::Log& new_log);

    virtual void start (Co::Engine& engine);
    virtual void stop  ();
    
    virtual void tick   (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count, const Co::KeyState* kb, v2f mouse_delta);
    virtual void render (Co::Frame& frame, float alpha);

  public:
    ~Game ()
    {
      model_factory.reset ();
      texture_factory.reset ();
      font_factory.reset ();
    }

    static Ptr create ()
    {
      return std::make_shared <Game> ();
    }

  };

  RK_MODULE_FACTORY (Game);

  void Game::init (Co::Engine& engine, Co::WorkQueue& queue, Co::RenderContext& rc, Co::Log& new_log)
  {
    log_ptr = &new_log;
    log () << "- SH-Game: Initializing\n";

    engine.get_object (texture_factory);
    engine.get_object (model_factory);
    engine.get_object (font_factory);

    world = World::create (queue, rc);
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
    pawn  -> tick (time, step, kb, mouse_delta);
    world -> tick (time, step, queue);
  }

  void Game::render (Co::Frame& frame, float alpha)
  {
    pawn  -> render (frame, alpha);
    world -> render (frame, alpha);
  }

} // namespace SH
