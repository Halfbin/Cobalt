//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_RENDERER
#define CO_H_RENDERER

// Implements
#include <Co/RenderDevice.hpp>

// Uses
#include <Co/WorkQueue.hpp>
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
    typedef std::shared_ptr <Renderer> Ptr;

    virtual void init         (void* hwnd) = 0;
    virtual void render_frame () = 0;

    virtual RenderContext& context () = 0;

  };

  class RendererRoot
  {
  public:
    virtual Renderer::Ptr create_renderer (Log& log, WorkQueue& queue, const Clock& clock) = 0;

  };

} // namespace Co

#endif
