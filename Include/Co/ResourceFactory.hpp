//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_RESOURCEFACTORY
#define CO_H_RESOURCEFACTORY

#include <unordered_map>

#include <Rk/Types.hpp>

namespace Co
{
  class BasicFactory
  {
  public:
    virtual u32 usage () const = 0;
    virtual u32 clear () = 0;

  };

  template <typename Key, typename Resource, typename Ptr = typename Resource::Ptr>
  class ResourceFactory :
    public BasicFactory
  {
    std::unordered_map <Key, Ptr> cache;

    virtual u32 usage () const
    {
      return cache.size ();
    };

    virtual u32 clear ()
    {
      for (auto iter = cache.begin (); iter != cache.end ();)
      {
        if (iter -> second.unique ())
          iter = cache.erase (iter);
        else
          iter++;
      }

      return usage ();
    }

  protected:
    Ptr find (const Key& key)
    {
      auto iter = cache.find (key);
      if (iter == cache.end ())
        return nullptr;
      else
        return iter -> second;
    }

    void add (Key key, Ptr ptr)
    {
      cache.emplace (
        std::make_pair (
          std::move (key),
          std::move (ptr)
        )
      );
    }

  };

}

#endif
