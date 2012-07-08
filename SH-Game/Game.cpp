//
// Copyright (C) 2011 Roadkill Software
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

namespace SH
{
  Co::ModelFactory::Ptr   model_factory;
  Co::TextureFactory::Ptr texture_factory;
  Co::FontFactory::Ptr    font_factory;

  Co::Log* log_ptr = 0;

  class Game :
    public Co::Game
  {

    virtual void init (Co::Engine& engine, Co::WorkQueue& queue, Co::Log& new_log);

    virtual Co::EntityClassBase& find_class (Rk::StringRef name);

    virtual void start (Co::Engine& engine);
    virtual void stop  ();
    
    virtual void tick   (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count);
    virtual void render (Co::Frame& frame, float alpha);

  public:
    static Ptr create ()
    {
      return std::make_shared <Game> ();
    }

  };

  RK_MODULE_FACTORY (Game);

  void Game::init (Co::Engine& engine, Co::WorkQueue& queue, Co::Log& new_log)
  {
    log_ptr = &new_log;
    log () << "- SH-Game: Initializing\n";

    engine.get_object (texture_factory);
    engine.get_object (model_factory);
    engine.get_object (font_factory);
  }

  extern Co::EntityClassBase
    & block_world_class,
    & test_entity_class,
    & spectator_class;

  Co::EntityClassBase& Game::find_class (Rk::StringRef name)
  {
    if (name == "BlockWorld")
      return block_world_class;
    else if (name == "TestEntity")
      return test_entity_class;
    else if (name == "Spectator")
      return spectator_class;

    throw std::invalid_argument ("No such class");
  }

  void Game::start (Co::Engine& engine)
  {
    log () << "- SH-Game: Starting\n";

    //engine.create_entity ("TestEntity");
    engine.create_entity ("BlockWorld");
    engine.create_entity ("Spectator" );

    engine.enable_ui (false);
  }

  void Game::stop ()
  {
    log () << "- SH-Game: Stopping\n";
  }

  void Game::tick (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count)
  { }

  void Game::render (Co::Frame& frame, float alpha)
  { }

} // namespace SH
