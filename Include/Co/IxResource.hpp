//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_IXRESOURCE
#define CO_H_IXRESOURCE

#include <Rk/IxShared.hpp>

namespace Co
{
  class IxRenderContext;

  class IxDisposable
  {
  public:
    virtual void dispose () = 0;

  };

  class IxResource :
    public Rk::IxShared,
    public IxDisposable
  {
  protected:
    bool ready;

    IxResource () :
      ready (false)
    { }
    
  public:
    typedef Rk::IxSharedPtr <IxResource> Ptr;

    virtual void load (IxRenderContext& rc) = 0;

  }; // class IxResource

} // namespace Co

#endif
