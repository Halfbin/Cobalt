//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXLOADCONTEXT
#define CO_H_IXLOADCONTEXT

#include <Rk/StringRef.hpp>

namespace Co
{
  class IxResource;
  class IxDisposable;
  class IxDataSource;

  class IxLoadContext
  {
  public:
    virtual void load    (IxResource*   resource) = 0;
    virtual void dispose (IxDisposable* garbage ) = 0;

    virtual Rk::StringRef get_game_path () = 0;
    //virtual IxDataSource* open (Rk::StringRef path) = 0;

  };

}

#endif
