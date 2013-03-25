//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_MODULEMANAGER
#define CO_H_MODULEMANAGER

#include <map>

#include <Rk/Module.hpp>

namespace Co
{
  class ModuleManager
  {
    std::map <std::string, Rk::Module> modules;

    void add (Rk::StringRef img_name, Rk::Module mod)
    {
      modules.insert (std::make_pair (img_name.string (), mod));
    }

  public:
    ~ModuleManager ()
    {
      flush ();
      //if (count () != 0)
        // uh-oh
    }

    Rk::Module find (Rk::StringRef img_name)
    {
      auto iter = modules.find (img_name.string ());
      if (iter != modules.end ())
        return iter -> second;
      else
        return nil;
    }

    template <typename Params>
    Rk::Module load (Rk::StringRef img_name, const Params& params)
    {
      Rk::Module mod = find (img_name);

      if (!mod)
      {
        mod.load (img_name.string () + CO_SUFFIX ".dll", params);
        add (img_name, mod);
      }
      
      return mod;
    }

    uptr count () const
    {
      return modules.size ();
    }

    void flush ()
    {
      for (auto iter = modules.begin (); iter != modules.end ();)
      {
        if (iter -> second.unique ())
        {
          modules.erase (iter);
          iter = modules.begin ();
        }
      }
    }

  };

}

#endif
