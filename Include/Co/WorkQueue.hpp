//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_WORKQUEUE
#define CO_H_WORKQUEUE

// Uses
#include <Rk/AsyncMethod.hpp>

#include <functional>
#include <memory>

namespace Co
{
  class RenderContext;
  class Filesystem;

  class WorkQueue
  {
  public:
    static Rk::StringRef ix_name () { return "Co::WorkQueue"; }
    typedef std::shared_ptr <WorkQueue> Ptr;

    virtual void work (RenderContext& rc, Filesystem& fs) = 0;
    virtual void stop () = 0;

    typedef std::function <void (WorkQueue&, RenderContext&, Filesystem&)>
      LoadFunc;
    typedef std::function <void ()>
      TrashFunc;

    virtual void queue_load  (LoadFunc  load) = 0;
    virtual void queue_trash (TrashFunc trash) = 0;

    template <typename T, typename Method>
    void queue_load (const std::shared_ptr <T>& ptr, Method&& method)
    {
      queue_load (Rk::async_method (ptr, std::forward <Method> (method)));
    }

    template <typename T, typename Nested>
    std::function <void (T*)> make_deleter (Nested nested)
    {
      return [this, nested] (T* p)
      {
        queue_trash ([nested, p] { nested (p); });
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

}

#endif
