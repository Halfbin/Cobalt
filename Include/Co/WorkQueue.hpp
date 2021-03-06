//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_WORKQUEUE
#define CO_H_WORKQUEUE

// Uses
#include <Co/Log.hpp>

#include <Rk/AsyncMethod.hpp>

#include <functional>
#include <memory>
//#include <tuple>

namespace Co
{
  class RenderContext;
  class AudioService;
  class Filesystem;
  
  struct LoadContext
  {
    RenderContext& rc;
    Filesystem&    fs;
    AudioService&  as;

    LoadContext (
      RenderContext& rc,
      Filesystem&    fs,
      AudioService&  as
    ) :
      rc (rc),
      fs (fs),
      as (as)
    { }
    
  };

  class WorkQueue
  {
  public:
    typedef std::shared_ptr <WorkQueue> Ptr;

    virtual void work           (LoadContext& ctx) = 0;
    virtual void do_completions () = 0;
    virtual void stop           () = 0;

    // Asynchronous jobs
    typedef std::function <void (WorkQueue&, LoadContext&)>
      LoadFunc;
    typedef std::function <void ()>
      TrashFunc, CompletionFunc;

    virtual void queue_load       (LoadFunc       load)  = 0;
    virtual void queue_trash      (TrashFunc      trash) = 0;
    virtual void queue_completion (CompletionFunc comp)  = 0;

    template <typename T, typename Method>
    void queue_load (const std::shared_ptr <T>& ptr, Method&& method)
    {
      queue_load (Rk::async_method (ptr, std::forward <Method> (method)));
    }

    template <typename Func, typename T>
    void queue_load (Func&& func, const std::shared_ptr <T>& ptr)
    {
      queue_load (Rk::async_method (ptr, std::forward <Method> (method)));
    }

    /*template <typename T, typename Method>
    void queue_completion (const std::shared_ptr <T>& ptr, Method&& method)
    {
      queue_completion (Rk::async_method (ptr, std::forward <Method> (method)));
    }*/

    // Resource management
    template <typename T, typename Nested>
    std::function <void (T*)> make_deleter (Nested nested)
    {
      return [this, nested] (T* p)
      {
        auto n = nested;
        queue_trash ([n, p] { n (p); });
      };
    }

    template <typename T>
    std::function <void (T*)> make_deleter ()
    {
      return [this] (T* p)
      {
        queue_trash ([p] { delete p; });
      };
    }

    template <typename T, typename Deleter>
    std::shared_ptr <T> gc_attach (T* raw, Deleter del)
    {
      return std::shared_ptr <T> (
        raw,
        make_deleter <T> (std::move (del))
      );
    }

    template <typename T>
    std::shared_ptr <T> gc_attach (T* raw)
    {
      return std::shared_ptr <T> (
        raw,
        make_deleter <T> ()
      );
    }

    template <typename T, typename Deleter>
    std::shared_ptr <T> gc_construct (T* raw, Deleter del)
    {
      auto ptr = gc_attach (raw, std::move (del));
      queue_load (ptr, &T::construct);
      return std::move (ptr);
    }

    template <typename T>
    std::shared_ptr <T> gc_construct (T* raw)
    {
      auto ptr = gc_attach (raw);
      queue_load (ptr, &T::construct);
      return std::move (ptr);
    }

  };

  class WorkQueueRoot
  {
  public:
    virtual WorkQueue::Ptr create_queue (Log& log) = 0;

  };

}

#endif
