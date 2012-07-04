//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_RENDERER
#define CO_H_RENDERER

// Implements
#include <Co/RenderDevice.hpp>

// Uses
#include <Co/Clock.hpp>
#include <Co/Frame.hpp>
#include <Co/Log.hpp>

#include <Rk/StreamForward.hpp>
#include <Rk/Types.hpp>

#include <memory>

namespace Co
{
  class Renderer :
    public RenderDevice
  {
  public:
    static Rk::StringRef ix_name () { return "Co::Renderer"; }
    typedef std::shared_ptr <Renderer> Ptr;

    virtual void   init        (void* hwnd, const Clock& clock, Log& log) = 0;
    virtual void   stop        () = 0;
    virtual Frame* begin_frame (float prev_time, float current_time) = 0;

  };

} // namespace Co

#endif
