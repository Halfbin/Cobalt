//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLRENDERER
#define CO_GLRENDERER_H_GLRENDERER

// Implements
#include <Co/IxRenderDevice.hpp>
#include <Co/IxRenderer.hpp>

// Uses
#include <Co/Log.hpp>

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
    public IxRenderer,
    public IxRenderDevice
  {
    // Frame queueing and rendering
    static const uint frame_count = 10;
    typedef Rk::FixedQueue <GLFrame*, frame_count> FrameQueue;

    GLFrame    frames [frame_count];
    FrameQueue free_frames;
    Rk::Mutex  free_frames_mutex;
    FrameQueue ready_frames;
    Rk::Mutex  ready_frames_mutex;
    GLFrame*   prepping_frame;
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
    Clock* clock;
    Log*   logger;

    // Shader
    u32 program,
        model_to_world,
        world_to_clip,
        world_to_eye;

    Log& log () { return *logger; }

    // Setup and teardown
    virtual void init         (void* new_target, Clock& new_clock, Log& new_logger);
    u32          load_shader  (const char* path, uint type);
    void         load_program ();
    void         cleanup      ();

    // Rendering control
    void         loop  ();
    virtual void start ();
    virtual void stop  ();

    // Frame exchange
    virtual Frame* begin_frame  (float prev_time, float current_time, u32& old_frame_id, u32& new_frame_id);
    virtual void   submit_frame (Frame*);

    // Device
    GLContext*               create_context_impl ();
    virtual IxRenderContext* create_context      ();
    virtual void             set_size            (uint width, uint height);

    GLRenderer  ();
    ~GLRenderer ();

  public:
    static GLRenderer instance;

    void expose (u64 ixid, void** result);

    void add_garbage_vao (u32 vao);

  }; // class GLRenderer

  static GLRenderer& renderer = GLRenderer::instance;

} // namespace Co

#endif
