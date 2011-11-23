//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/IxLoader.hpp>

// Uses
#include <Co/IxRenderContext.hpp>
#include <Co/IxRenderDevice.hpp>
#include <Co/IxResource.hpp>
#include <Co/Log.hpp>

#include <Rk/Condition.hpp>
#include <Rk/Expose.hpp>
#include <Rk/Thread.hpp>
#include <Rk/Mutex.hpp>

#include <deque>

namespace Co
{
namespace
{
  //
  // = Loader ==========================================================================================================
  //
  class Loader :
    public IxLoader
  {
    //
    // Garbage
    // Necessary for synchronizing resource cleanup with the renderer
    //
    struct Garbage
    {
      IxDisposable* disposable;
      u32           soonest_frame;

      Garbage () :
        disposable    (0),
        soonest_frame (0)
      { }
      
      Garbage (IxDisposable* disp, u32 id) :
        disposable    (disp),
        soonest_frame (id)
      { }
      
    };

    // Job queues
    typedef std::deque <IxResource::Ptr> Queue;
    typedef std::deque <Garbage>         GarbageQueue;
    Queue                                queue;
    GarbageQueue                         garbage_queue;

    // Execution control
    Rk::Mutex     mutex;
    Rk::Condition pending;
    Rk::Thread    thread;
    bool          run,
                  collect_garbage;

    // Renderer synchronization
    u32 current_frame,
        newest_finished_frame;

    // Info
    Rk::ShortString <512> game_path;

    // Subsystems
    IxRenderDevice* device;
    Log*            logger;

    Log& log () { return *logger; }

    // Thread function
    void loop ();
    
    // Initialization
    virtual void init (IxRenderDevice& device, Log& new_logger, Rk::StringRef new_game_path);

    // Control
    virtual void start ();
    virtual void stop  ();
    virtual void cancel_all ();
    virtual void collect ();

    // Job queueing
    virtual void load    (IxResource*   resource);
    virtual void dispose (IxDisposable* garbage );

    // Load context info
    virtual Rk::StringRef get_game_path ();
    //virtual IxDataSource* open (Rk::StringRef path);

  public:
    Loader ();
    //~Loader ();

  } loader;

  Loader::Loader ()
  {
    run = false;
  }

  /*Loader::~Loader ()
  {
    
  }*/

  void Loader::init (IxRenderDevice& rd, Log& new_logger, Rk::StringRef new_game_path)
  {
    Rk::require (!device, "Loader already initialized");

    game_path = new_game_path;
    device = &rd;
    logger = &new_logger;

    log () << "- Loader initialized\n";
  }

  void Loader::loop ()
  {
    log () << "- Loader running\n";

    IxRenderContext::Ptr rc = device -> create_context ();

    std::deque <IxResource::Ptr> temp_queue;
    std::deque <Garbage>         temp_garbage_queue;

    for (;;)
    {
      auto lock = mutex.get_lock ();

      if (!run && garbage_queue.empty ())
        return;

      while (queue.empty () && !collect_garbage && run)
        pending.wait (lock);

      if (run)
        temp_queue = std::move (queue);

      if (collect_garbage)
      {
        if (run)
        {
          auto collect_range_end = garbage_queue.begin ();
          while (collect_range_end != garbage_queue.end () && collect_range_end -> soonest_frame <= newest_finished_frame)
            collect_range_end++;
          temp_garbage_queue.assign (garbage_queue.begin (), collect_range_end);
          garbage_queue.erase       (garbage_queue.begin (), collect_range_end);
        }
        else
        {
          temp_garbage_queue = std::move (garbage_queue);
        }
        
        collect_garbage = false;
      }

      lock = nil;

      for (auto r = temp_queue.begin (); r != temp_queue.end (); r++) try
      {
        (*r) -> load (*rc);
      }
      catch (const std::exception& e)
      {
        log () << e.what () << '\n'
               << "X Caught in Loader\n";
      }
      catch (...)
      {
        log () << "X Unknown exception\n"
               << "X Caught in Loader\n";
      }

      for (auto g = temp_garbage_queue.begin (); g != temp_garbage_queue.end (); g++)
        g -> disposable -> dispose ();
        
      rc -> flush ();

      temp_queue.clear ();
      temp_garbage_queue.clear ();
    }
  }

  void Loader::start ()
  {
    Rk::require (device != 0, "Loader must be initialized before being started");

    if (run)
      return;

    run = true;

    log () << "- Loader starting\n";

    thread.execute (
      [this] { loop (); }
    );
  }

  void Loader::stop ()
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

    log () << "- Loader stopping\n";

    thread.join ();
  }

  void Loader::load (IxResource* resource)
  {
    auto lock = mutex.get_lock ();
    resource -> acquire ();
    queue.push_back (resource);
    pending.notify_one ();
  }

  void Loader::dispose (IxDisposable* resource)
  {
    auto lock = mutex.get_lock ();
    garbage_queue.push_back (Garbage (resource, current_frame));
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

  Rk::StringRef Loader::get_game_path ()
  {
    return game_path;
  }

} // namespace
  
  IX_EXPOSE (void** out, u64 ixid)
  {
    Rk::expose <IxLoader> (loader, ixid, out);
  }

} // namespace Co
