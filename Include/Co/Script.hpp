//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_SCRIPT
#define CO_H_SCRIPT

#include <Co/WorkQueue.hpp>
#include <Co/Log.hpp>

#include <memory>

namespace Co
{
  class ScriptContext
  {
  public:
    typedef std::shared_ptr <ScriptContext> Ptr;

  };

  class ScriptFactory
  {
  public:
    typedef std::shared_ptr <ScriptFactory> Ptr;

    virtual ScriptContext::Ptr create () = 0;

  };

  class ScriptRoot
  {
  public:
    virtual ScriptFactory::Ptr create_factory (Log& log, WorkQueue& queue) = 0;

  };

} // namespace Co

#endif
