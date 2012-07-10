//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

/*#ifndef CO_SERVER_H_SERVER
#define CO_SERVER_H_SERVER

#include <Co/Game.hpp>
#include <Co/Clock.hpp>

#include <Rk/StringRef.hpp>
#include <Rk/Module.hpp>
#include <Rk/Types.hpp>

namespace Co
{
  class Game;
  
  class Server
  {
    MasterClock clock;
    Game::Ptr   game;
    
    void cleanup ();

  public:
    Server (Rk::StringRef config_path);
    ~Server ();

    void run ();

  }; // class Server

} // namespace Co

#endif
*/