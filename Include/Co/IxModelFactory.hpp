//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXMODELFACTORY
#define CO_H_IXMODELFACTORY

#include <Rk/StringRef.hpp>

namespace Co
{
  class IxLoadContext;
  class IxModel;

  class IxModelFactory
  {
  public:
    static const u64 id = 0x7d39d77531907521ull;

    virtual IxModel* create (IxLoadContext& context, Rk::StringRef path) = 0;

  };

} // namespace Co

#endif
