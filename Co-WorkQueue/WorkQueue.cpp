//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/WorkQueue.hpp>

// Uses
#include <Co/RenderContext.hpp>

#include <Rk/Condition.hpp>
#include <Rk/Modular.hpp>
#include <Rk/Mutex.hpp>

#include <vector>
#include <deque>

namespace Co
{
  class QueueImpl :
    public WorkQueue
  {
    Log&                         log;
    std::deque <LoadFunc>        load_queue;
    std::deque <TrashFunc>       trash_queue;
    std::vector <CompletionFunc> completion_queue;
    Rk::Mutex                    mutex,
                                 completion_mutex;
    Rk::Condition                job_queued;
    bool                         run;

    virtual void work           (LoadContext& ctx);
    virtual void do_completions ();
    virtual void stop           ();

    virtual void queue_load       (LoadFunc       load);
    virtual void queue_trash      (TrashFunc      trash);
    virtual void queue_completion (CompletionFunc comp);

  public:
    QueueImpl (Log&);
    ~QueueImpl ();

  };

  void QueueImpl::work (LoadContext& ctx)
  {
    auto lock = mutex.get_lock ();

    while (run || !trash_queue.empty ())
    {
      while (trash_queue.empty () && load_queue.empty () && run)
      {
        ctx.rc.flush ();
        job_queued.wait (lock);
      }

      if (run && !load_queue.empty ())
      {
        auto job = load_queue.front ();
        load_queue.pop_front ();
        lock = nil;
        job (*this, ctx);
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

  void QueueImpl::do_completions ()
  {
    std::vector <CompletionFunc> comps;

    {
      auto lock = completion_mutex.get_lock ();
      std::swap (comps, completion_queue);
    }

    for (auto comp = comps.begin (); comp != comps.end (); comp++)
      (*comp) ();
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

  void QueueImpl::queue_completion (CompletionFunc comp)
  {
    auto lock = completion_mutex.get_lock ();
    completion_queue.push_back (std::move (comp));
  }

  QueueImpl::QueueImpl (Log& log) :
    log (log),
    run (true)
  { }
  
  QueueImpl::~QueueImpl ()
  {
    trash_queue.clear ();
    load_queue.clear ();
    completion_queue.clear ();
  }

  class Root :
    public WorkQueueRoot
  {
    virtual WorkQueue::Ptr create_queue (Log& log)
    {
      return std::make_shared <QueueImpl> (log);
    }

  };

  RK_MODULE (Root);
  
}
