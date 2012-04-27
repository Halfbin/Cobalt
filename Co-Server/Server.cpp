//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//
/*
// Implements
#include "Server.hpp"

// Uses
#include <Rk/StringOutStream.hpp>
#include <Rk/ShortString.hpp>

#include "Net.hpp"

namespace Co
{
  Server::Server (Rk::StringRef config_path)
  {
    Rk::StringRef game_name = "Iridium";

    // Parse configuration
    Rk::StringOutStream game_path;
    game_path << "../" << game_name << "/";

    // Load subsystem modules
    Rk::load_module (game, game_path.string () + "Binaries/Co-Game" CO_SUFFIX ".dll");

    // Initialize subsystems
    game -> init (engine.get (), loader, log.get_impl ());
    if (!ok)
      throw std::runtime_error ("Co-Client: Game::init - error initializing game");

    net.init ();
  }

  Server::~Server ()
  {
    net.shutdown ();
    game -> stop ();
  }

  void Server::run ()
  {
    game -> start ();
    net.open (30355);
    
  } // run

} // namespace Co
*/