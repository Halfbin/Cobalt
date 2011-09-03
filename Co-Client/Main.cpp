//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/Log.hpp>
#include "Client.hpp"

namespace Co
{
  Log log ("Co-Client" CO_SUFFIX ".log");

  extern "C" int __stdcall WinMain (void*, void*, char*, int)
  {
    log << "- Co-Client" CO_SUFFIX " starting\n";

    try
    {
      Client client ("./");
      client.run ();
    }
    catch (const std::exception& e)
    {
      log << e.what () << '\n';
    }
    catch (...)
    {
      log << "Exception caught\n";
    }

    log << "- Co-Client" CO_SUFFIX " exiting\n";
    return 0;
  }

} // namespace Co
