//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENTFRONTEND_H_CLIENTWINDOW
#define CO_CLIENTFRONTEND_H_CLIENTWINDOW

#include <Rk/StringRef.hpp>
#include <Rk/Types.hpp>

#include <functional>

namespace Co
{
  struct Rect
  {
    i32 left, top, right, bottom;

    Rect ()
    { }

    Rect (i32 l, i32 t, i32 r, i32 b) :
      left (l), top (t), right (r), bottom (b)
    { }
      
  };

  class ClientWindow
  {
    static iptr __stdcall message_proxy   (void*, u32, uptr, iptr);
    iptr                  message_handler (u32, uptr, iptr);

  public:
    typedef std::function <iptr (ClientWindow*, u32, uptr, iptr)> Handler;

  private:
    void*   handle;
    Handler handler;
    Rect    borders;
    uint    width,
            height;
    bool    fullscreen;
    Rect    win_rect;
    bool    win_maxed;

  public:
    ClientWindow (const Nil& n = nil) :
      handle     (0),
      width      (0),
      height     (0),
      fullscreen (false),
      win_rect   (0, 0, 0, 0),
      win_maxed  (false)
    { }
    
    void create (Rk::WStringRef title, Handler handler, bool fullscreen, uint window_width, uint window_height);
    
    ClientWindow (Rk::WStringRef title, Handler handler, bool fullscreen, uint window_width, uint window_height) :
      handle     (0),
      width      (0),
      height     (0),
      fullscreen (false),
      win_rect   (0, 0, 0, 0),
      win_maxed  (false)
    {
      create (title, handler, fullscreen, window_width, window_height);
    }
    
    bool is_fullscreen () const
    {
      return fullscreen;
    }

    void set_fullscreen (bool fullscreen);

    ~ClientWindow ();

    void* get_handle ()
    {
      return handle;
    }

    iptr message_default (u32, uptr, iptr);

    void show (bool value = true);

    void hide ()
    {
      show (false);
    }

    uint get_width () const
    {
      return width;
    }

    uint get_height () const
    {
      return height;
    }

  };

} // namespace Co

#endif
