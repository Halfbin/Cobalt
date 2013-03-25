//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Co/ClientFrontend.hpp>

namespace Pyr
{
  Co::GameClient::Ptr create_title (Co::ClientFrontend& frontend);

  extern "C" int __stdcall WinMain (void*, void*, char*, int) try
  {
    Co::ClientFrontend frontend (
      "Pyrite",
      L"Pyrite",
      false,
      1280,
      720
    );

    frontend.run (create_title (frontend));

    return 0;
  }
  catch (...)
  {
    return 1;
  }

}
