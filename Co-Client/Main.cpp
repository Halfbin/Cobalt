//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#include <Rk/FileOutStream.hpp>

#include "Client.hpp"

#include <vector>
#include <map>

namespace Co
{
  Rk::FileOutStream log_file ("Co-Client" CO_SUFFIX ".log", Rk::File::open_replace_or_create);
  Rk::OutStream log_stream = log_file.virtualize ();
  Log log (log_stream);

  static std::vector <Rk::Module>     modules;
  static std::map <std::string, uptr> lookup;

  Rk::Module load_module (Rk::StringRef path)
  {
    auto iter = lookup.find (path.string ());
    if (iter != lookup.end ())
      return modules [iter -> second];

    Rk::Module mod (path.string () + CO_SUFFIX ".dll");
    lookup.insert (std::make_pair (path.string (), modules.size ()));
    modules.push_back (mod);
    return mod;
  }

  extern "C" int __stdcall WinMain (void*, void*, char*, int)
  {
    log_file << "- Co-Client" CO_SUFFIX " starting\n"
             << "* Starting initialization\n";

    try
    {
      Client client ("./");
      client.run ();
    }
    catch (...)
    {
      log_exception (log, "in main thread");
    }

    // Unload modules in reverse order
    while (!modules.empty ())
      modules.pop_back ();

    log_file << "* Shutdown complete\n"
             << "- Co-Client" CO_SUFFIX " exiting\n";

    log_file.flush ();

    return 0;
  }

} // namespace Co
