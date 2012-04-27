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
    
    virtual void tick (
      Co::WorkQueue&     queue,
      Co::Frame&         frame,
      float              cur_time,
      float              prev_time,
      const Co::UIEvent* ui_events,
      uint               ui_event_count
    );

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

  Co::EntityClassBase& Game::find_class (Rk::StringRef name)
  {
    throw std::invalid_argument ("No such class");
  }

  void Game::start (Co::Engine& engine)
  {
    log () << "- SH-Game: Starting\n";

    /*engine.create_entity ("TestEntity", Co::PropMap ());
    engine.create_entity ("BlockWorld", Co::PropMap ());
    engine.create_entity ("Spectator",  Co::PropMap ());*/
  }

  void Game::stop ()
  {
    log () << "- SH-Game: Stopping\n";
  }

  void Game::tick (
    Co::WorkQueue&     queue,
    Co::Frame&         frame,
    float              cur_time,
    float              prev_time,
    const Co::UIEvent* ui_events,
    uint               ui_event_count)
  {

  }

} // namespace SH
