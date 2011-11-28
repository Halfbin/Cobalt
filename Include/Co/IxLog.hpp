//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_LOG
#define CO_H_LOG

#include <Rk/VirtualOutStream.hpp>
#include <Rk/StringRef.hpp>
#include <Rk/IxUnique.hpp>

#ifndef CO_COMMON_API
#define CO_COMMON_API __declspec (dllimport)
#endif

namespace Co
{
  class IxLog :
    public Rk::IxLockedOutStreamImpl,
    public Rk::IxUnique
  {
  public:
    typedef Rk::IxUniquePtr <IxLog> Ptr;

  };

  CO_COMMON_API IxLog* create_log (Rk::StringRef path);

} // namespace Co

#endif
