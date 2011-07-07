//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_CLIENT_H_GLWINDOW
#define CO_CLIENT_H_GLWINDOW

#include <Rk/Types.hpp>

namespace Co
{
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
    uint    width,
            height;

  public:
    GLWindow (const Nil& n = nil) :
      handle (0),
      user   (0),
      width  (0),
      height (0)
    { }
    
    void create (const wchar_t* title, Handler handler, bool fullscreen, uint window_width, uint window_height, void* user = 0);
    
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

  };

} // namespace Co

#endif
