//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENT_H_GLWINDOW
#define CO_CLIENT_H_GLWINDOW

#include <Rk/Types.hpp>

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

  class GLWindow
  {
    static iptr __stdcall message_proxy   (void*, u32, uptr, iptr);
    iptr                  message_handler (u32, uptr, iptr);

  public:
    typedef iptr (*Handler) (GLWindow*, u32, uptr, iptr);

  private:
    void*   handle;
    void*   user;
    Handler handler;
    Rect    borders;
    uint    width,
            height;
    bool    fullscreen;
    Rect    win_rect;
    bool    win_maxed;

  public:
    GLWindow (const Nil& n = nil) :
      handle     (0),
      user       (0),
      width      (0),
      height     (0),
      fullscreen (false),
      win_rect   (0, 0, 0, 0),
      win_maxed  (false)
    { }
    
    void create (const wchar_t* title, Handler handler, bool fullscreen, uint window_width, uint window_height, void* user = 0);
    
    bool is_fullscreen () const
    {
      return fullscreen;
    }

    void set_fullscreen (bool fullscreen);

    ~GLWindow ();

    void* get_handle ()
    {
      return handle;
    }

    void* get_user () const
    {
      return user;
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
