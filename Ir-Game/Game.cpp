//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Game.hpp>

// Uses
#include <Co/Engine.hpp>
#include <Co/Frame.hpp>

#include <Rk/Module.hpp>

#include "Common.hpp"
#include "State.hpp"

namespace Ir
{
  State* State::current = 0;
  State* State::next    = 0;
  bool   State::loading = false;

  Co::Log* log_ptr = 0;

  Co::TextureFactory::Ptr texture_factory;
  Co::ModelFactory::Ptr   model_factory;
  Co::FontFactory::Ptr    font_factory;
  
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
    log () << "- Ir-Game: Initializing\n";

    engine.get_object (texture_factory);
    engine.get_object (model_factory);
    engine.get_object (font_factory);
  }

  void Game::start (Co::Engine& engine)
  {
    log () << "- Ir-Game: Starting\n";
    State::init (title_state);
  }

  void Game::stop ()
  {
    log () << "- Ir-Game: Stopping\n";
    State::finish ();
  }

  void Game::tick (float time, float step, Co::WorkQueue& queue, const Co::UIEvent* ui_events, uint ui_event_count)
  {
    State::tick_current (time, step, queue, ui_events, ui_event_count);
  }
  
  void Game::render (Co::Frame& frame, float alpha)
  {
    State::render_current (frame, alpha);
  }

  Co::EntityClassBase& Game::find_class (Rk::StringRef name)
  {
    throw std::invalid_argument ("No such class");
  }

  void State::show_loading_screen (Co::Frame& frame, float time, float prev_time)
  {
    /*Co::TexRect square (
      -10, -10, 20, 20,
        0,   0,  1,  1
    );

    v4f col (1.0f, 1.0f, 1.0f, 1.0f);

    frame.add_label (
      nullptr,
      &square, 1,
      Co::Spatial2D (v2f (20, 20), Co::rotate (prev_time * 5.0f)),
      Co::Spatial2D (v2f (20, 20), Co::rotate (time      * 5.0f)),
      nil, nil,
      col, col
    );*/
  }

} // namespace Ir
