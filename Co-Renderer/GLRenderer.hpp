//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLRENDERER
#define CO_GLRENDERER_H_GLRENDERER

// Implements
#include <Co/Renderer.hpp>

// Uses
#include <Rk/FixedQueue.hpp>
#include <Rk/Thread.hpp>
#include <Rk/Mutex.hpp>

#include "GLContext.hpp"
#include "GLFrame.hpp"

namespace Co
{
  //
  // = GLRenderer ======================================================================================================
  //
  class GLRenderer :
    public Renderer
  {
    // Frame queueing and rendering
    static const uint frame_count = 10;
    typedef Rk::FixedQueue <GLFrame*, frame_count> FrameQueue;

    GLFrame    frames [frame_count];
    FrameQueue free_frames;
    Rk::Mutex  free_frames_mutex;
    FrameQueue ready_frames;
    Rk::Mutex  ready_frames_mutex;
    Rk::Thread thread;

    // Render device
    void*     target;
    void*     shared_dc;
    int       format;
    void*     shared_rc;
    WGLCCA    wglCreateContextAttribs;
    Rk::Mutex device_mutex;
    uint      width,
              height;

    // Subsystems
    const Clock* clock;
    Log*         log_ptr;

    // Internal
    void loop  ();
    
    // Setup and teardown
    virtual void init    (void* hwnd, const Clock& clock, Log& log);
    void         cleanup ();
    virtual void stop    ();

    // Frame exchange
    virtual Frame* begin_frame (
      float prev_time,
      float current_time
    );

    // Device
    GLContext::Ptr             create_context_impl ();
    virtual RenderContext::Ptr create_context      ();
    virtual void               set_size            (uint width, uint height);

    GLRenderer ();

  public:
    Log::Lock log ()
    {
      return log_ptr -> lock ();
    }

    static GLRenderer instance;

    static std::shared_ptr <GLRenderer> create ();

    void submit_frame (GLFrame* frame);

  }; // class GLRenderer

  static GLRenderer& renderer = GLRenderer::instance;

} // namespace Co

#endif
