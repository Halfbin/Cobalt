//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxLoader.hpp>

// Uses
#include <Co/IxRenderDevice.hpp>
#include <Co/IxResource.hpp>

#include <Rk/Condition.hpp>
#include <Rk/Thread.hpp>
#include <Rk/Mutex.hpp>

#include <deque>

namespace Co
{
namespace
{
  class Loader :
    public IxLoader
  {
    struct Garbage
    {
      IxDisposable* disposable;
      u32           soonest_frame;
    };

    typedef std::deque <IxResource::Ptr> Queue;
    typedef std::deque <Garbage>         GarbageQueue;

    Queue           queue;
    GarbageQueue    garbage;
    u32             current_frame;
    Rk::Mutex       mutex;
    Rk::Condition   pending;
    Rk::Thread      thread;
    bool            run,
                    collect_garbage;
    u32             newest_finished_frame;
    IxRenderDevice* device;

    virtual void init (IxRenderDevice* device);

    void loop ();

    virtual void start ();
    virtual void stop  ();

    virtual void cancel_all ();
    virtual void collect ();

  public:
    Loader ();
    ~Loader ();

  } loader;

  Loader::Loader ()
  {
    run = false;
  }

  Loader::~Loader ()
  {
    
  }

  void Loader::init (IxRenderDevice* rd)
  {
    device = rd;
  }

  void Loader::loop ()
  {
    IxRenderContext::Ptr rc = device -> create_context ();

    std::deque <IxResource::Ptr> temp_queue;
    std::deque <Garbage>         temp_garbage;

    for (;;)
    {
      auto lock = mutex.get_lock ();

      if (!run && garbage.empty ())
        return;

      while (queue.empty () && !collect_garbage && run)
        pending.wait (lock);

      if (run)
        temp_queue = std::move (queue);

      if (collect_garbage)
      {
        if (run)
        {
          auto collect_range_end = garbage.begin ();
          while (collect_range_end != garbage.end () && collect_range_end -> soonest_frame <= newest_finished_frame)
            collect_range_end++;
          temp_garbage.assign (garbage.begin (), collect_range_end);
          garbage.erase       (garbage.begin (), collect_range_end);
        }
        else
        {
          temp_garbage = std::move (garbage);
        }
        
        collect_garbage = false;
      }

      lock = nil;
        for (auto r = temp_queue.begin (); r != temp_queue.end (); r++) try
        {
          (*r) -> load (rc);
        }
        catch (...) { Rk::log_frame ("loader_loop"); }
        
        for (auto g = temp_garbage.begin (); g != temp_garbage.end (); g++)
          g -> disposable -> dispose ();
        
        temp_queue.clear ();
        temp_garbage.clear ();
    }
  }

  void Loader::start ()
  {
    if (run)
      return;

    run = true;

    thread.execute (
      [this] { loop (); }
    );
  }

  void Loader::stop  ()
  {
    if (!run)
      return;

    {
      auto lock = mutex.get_lock ();
      queue.clear ();
      run = false;
      collect_garbage = true;
      pending.notify_all ();
    }

    thread.join ();
  }

  void Loader::cancel_all ()
  {
    auto lock = mutex.get_lock ();
    queue.clear ();
  }

  void Loader::collect ()
  {
    auto lock = mutex.get_lock ();
    collect_garbage = true;
    pending.notify_one ();
  }

} // namespace
  
  extern "C" __declspec(dllexport) void* __cdecl ix_expose (u64 ixid)
  {
    if (ixid == IxLoader::id) return &loader;
    else return 0;
  }

} // namespace Co
