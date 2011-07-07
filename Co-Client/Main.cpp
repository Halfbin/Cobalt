//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/Log.hpp>
#include "Client.hpp"

namespace Co
{
  Log log ("Co-Client" CO_SUFFIX ".log");

  extern "C" int __stdcall WinMain (void*, void*, char*, int) try
  {
    Rk::Exception::stream = &log;
    log << "- Co-Client starting\n";

    Client client ("./");
    client.run ();

    return 0;
  }
  catch (...)
  {
    Rk::log_frame ("WinMain");
    return 1;
  }

}
