//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_MODULEMANAGER
#define CO_H_MODULEMANAGER

#include <Co/Log.hpp>

#include <Rk/Modular.hpp>

#include <map>

namespace Co
{
  class ModuleManager
  {
    Log& log;

    std::map <std::string, Rk::Module> modules;

    void add (const std::string& img_path, Rk::Module mod)
    {
      modules.insert (std::make_pair (img_path, mod));
    }

  public:
    ModuleManager (Log& log) :
      log (log)
    { }
    
    ~ModuleManager ()
    {
      flush ();
      //if (count () != 0)
        // uh-oh
    }

    Rk::Module find (Rk::StringRef img_name)
    {
      auto img_path = img_name.string () + CO_SUFFIX ".dll";
      auto iter = modules.find (img_path);
      if (iter != modules.end ())
        return iter -> second;
      else
        return nil;
    }

    Rk::Module load (Rk::StringRef img_name)
    {
      auto img_path = img_name.string () + CO_SUFFIX ".dll";
      Rk::Module mod = find (img_name);

      if (!mod)
      {
        log () << "- Loading \"" << img_path << "\"\n";
        mod.load (img_path);
        add (img_path, mod);
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
          log () << "- Unloading \"" << iter -> first << "\"\n";
          modules.erase (iter);
          iter = modules.begin ();
        }
      }
    }

  };

}

#endif
