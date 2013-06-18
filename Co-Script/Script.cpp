//
// Copyright (C) 2013 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Script.hpp>

// Uses
#include <Rk/Modular.hpp>

namespace Co
{
  //
  // = ContextImpl =====================================================================================================
  //
  class ContextImpl :
    public ScriptContext
  {
  public:
    ContextImpl ()
    {

    }

  };

  //
  // = FactoryImpl =====================================================================================================
  //
  class FactoryImpl :
    public ScriptFactory
  {
    Log&       log;
    WorkQueue& queue;

    virtual ScriptContext::Ptr create ()
    {
      return std::make_shared <ContextImpl> ();
    }

  public:
    FactoryImpl (Log& log, WorkQueue& queue) :
      log   (log),
      queue (queue)
    { }
    
  };

  class Root :
    public ScriptRoot
  {
    virtual ScriptFactory::Ptr create_factory (Log& log, WorkQueue& queue)
    {
      return std::make_shared <FactoryImpl> (log, queue);
    }

  };

  RK_MODULE (Root);

} // namespace Co
