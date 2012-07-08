//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_GAME
#define CO_H_GAME

// Uses
#include <Co/EntityClass.hpp>
#include <Co/WorkQueue.hpp>
#include <Co/UIInput.hpp>
#include <Co/Frame.hpp>
#include <Co/Log.hpp>

#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  class Engine;

  class Game
  {
  public:
    static Rk::StringRef ix_name () { return "Co::Game"; }
    typedef std::shared_ptr <Game> Ptr;

    virtual void init (Co::Engine& engine, Co::WorkQueue& queue, Log& new_log) = 0;

    virtual EntityClassBase& find_class (Rk::StringRef name) = 0;

    virtual void start (Co::Engine& engine) = 0;
    virtual void stop  () = 0;
    
    virtual void tick   (float time, float step, WorkQueue& queue, const UIEvent* ui_events, uint ui_event_count) = 0;
    virtual void render (Frame& frame, float alpha) = 0;

  };

} // namespace Co

#endif
