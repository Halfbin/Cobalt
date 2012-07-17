//
// Copyright (C) 2012 Roadkill Software
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
    public RenderDevice,
    public Frame
  {
  public:
    static Rk::StringRef ix_name () { return "Co::Renderer"; }
    typedef std::shared_ptr <Renderer> Ptr;

    virtual void init         (void* hwnd, const Clock& clock, Log& log) = 0;
    virtual void render_frame () = 0;

  };

} // namespace Co

#endif
