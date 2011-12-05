//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/IxLog.hpp>
#include "Client.hpp"

namespace Co
{
  IxLog::Ptr log_ptr = create_log ("Co-Client" CO_SUFFIX ".log");
  Rk::VirtualLockedOutStream log (log_ptr.get ());

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
      log << "X " << e.what () << '\n';
    }
    catch (...)
    {
      log << "X Exception caught\n";
    }

    log << "- Co-Client" CO_SUFFIX " exiting\n";
    return 0;
  }

} // namespace Co
