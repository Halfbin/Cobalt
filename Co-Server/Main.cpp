//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

/*#include <Co/Log.hpp>
#include "Server.hpp"*/

namespace Co
{
  /*Log::Ptr log_ptr = create_log ("Co-Server" CO_SUFFIX ".log");
  Rk::VirtualLockedOutStream log (log_ptr.get ());*/

  extern "C" int __stdcall WinMain (void*, void*, char*, int)
  {
    /*log << "- Co-Server" CO_SUFFIX " starting\n";

    try
    {
      Server server ("./");
      server.run ();
    }
    catch (const std::exception& e)
    {
      log << "X " << e.what () << '\n';
    }
    catch (...)
    {
      log << "X Exception caught\n";
    }

    log << "- Co-Server" CO_SUFFIX " exiting\n";*/
    return 0;
  }

} // namespace Co
