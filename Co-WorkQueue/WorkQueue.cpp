//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/WorkQueue.hpp>

// Uses
#include <Rk/Condition.hpp>
#include <Rk/Module.hpp>
#include <Rk/Mutex.hpp>

#include <deque>

namespace Co
{
  class QueueImpl :
    public WorkQueue
  {
    std::deque <LoadFunc>  load_queue;
    std::deque <TrashFunc> trash_queue;
    Rk::Mutex              mutex;
    Rk::Condition          job_queued;
    bool                   run;

    virtual void work (RenderContext& rc, Filesystem& fs);
    virtual void stop ();

    virtual void queue_load  (LoadFunc  load);
    virtual void queue_trash (TrashFunc trash);

  public:
    QueueImpl ();

    static Ptr create ()
    {
      return std::make_shared <QueueImpl> ();
    }

  };

  RK_MODULE_FACTORY (QueueImpl);

  void QueueImpl::work (RenderContext& rc, Filesystem& fs)
  {
    auto lock = mutex.get_lock ();

    while (run || !trash_queue.empty ())
    {
      while (trash_queue.empty () && load_queue.empty () && run)
        job_queued.wait (lock);

      if (run && !load_queue.empty ())
      {
        auto job = load_queue.front ();
        load_queue.pop_front ();
        lock = nil;
        job (*this, rc, fs);
      }
      else if (!trash_queue.empty ())
      {
        auto trash = trash_queue.front ();
        trash_queue.pop_front ();
        lock = nil;
        trash ();
      }

      lock = mutex.get_lock ();
    }
  }

  void QueueImpl::stop ()
  {
    auto lock = mutex.get_lock ();
    run = false;
    job_queued.notify_all ();
  }

  void QueueImpl::queue_load (LoadFunc job)
  {
    auto lock = mutex.get_lock ();
    load_queue.push_back (std::move (job));
    job_queued.notify_one ();
  }

  void QueueImpl::queue_trash (TrashFunc trash)
  {
    auto lock = mutex.get_lock ();
    trash_queue.push_back (std::move (trash));
    job_queued.notify_one ();
  }

  QueueImpl::QueueImpl () :
    run (true)
  { }
  
}
