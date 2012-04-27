//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLWindow.hpp"

// Uses
#include <Rk/Exception.hpp>

namespace Co
{
  typedef iptr (__stdcall *WndProc) (void*, u32, uptr, iptr);

  struct WndClassExW
  {
    u32           size,
                  style;
    WndProc       window_proc;
    u32           class_extra,
                  window_extra;
    void          *instance,
                  *icon,
                  *cursor,
                  *background_brush;
    const wchar_t *menu_name,
                  *class_name;
    void*         small_icon;
  };

  struct Point
  {
    i32 x, y;
  };

  struct WindowPlacement
  {
    u32   size,
          flags,
          show;
    Point min_pos,
          max_pos;
    Rect  rect;
  };

  enum
  {
    ws_maximizebox  = 0x00010000L,
    ws_minimizebox  = 0x00020000L,
    ws_thickframe   = 0x00040000L,
    ws_sysmenu      = 0x00080000L,
    ws_border       = 0x00800000L,
    ws_dlgframe     = 0x00400000L,
    ws_clipchildren = 0x02000000L,
    ws_clipsiblings = 0x04000000L,
    ws_popup        = 0x80000000L,
    ws_caption      = ws_border | ws_dlgframe,
    
    ws_ex_topmost = 0x00000008L,
    
    ws_maximize = 0x01000000,

    idi_application = 32512,
    idc_arrow = 32512,
    
    gwlp_userdata = -21,

    sm_cxscreen = 0,
    sm_cyscreen = 1,
    
    sw_hide     = 0,
    sw_show     = 5,
    sw_maximize = 3,

    wm_size = 5,

    windowed_style      = ws_clipsiblings | ws_caption | ws_thickframe | ws_sysmenu | ws_minimizebox | ws_maximizebox,
    windowed_style_ex   = 0,
    fullscreen_style    = ws_clipsiblings | ws_popup,
    fullscreen_style_ex = ws_ex_topmost,

    gwl_style   = -16,
    gwl_exstyle = -20,

    black_brush = 4
  };

  extern "C"
  {
    __declspec(dllimport) long  __stdcall GetWindowLongW     (void*, i32);
    __declspec(dllimport) long  __stdcall SetWindowLongW     (void*, i32, long);
  #ifdef _WIN64
    __declspec(dllimport) void* __stdcall GetWindowLongPtrW  (void*, i32);
    __declspec(dllimport) void* __stdcall SetWindowLongPtrW  (void*, i32, void*);
  #else
    static inline void* GetWindowLongPtrW (void* h, i32 i)          { return (void*) GetWindowLongW (h, i);    }
    static inline void* SetWindowLongPtrW (void* h, i32 i, void* p) { return (void*) SetWindowLongW (h, i, (long) p); }
  #endif
    __declspec(dllimport) iptr  __stdcall DefWindowProcW     (void*, u32, uptr, iptr);
    __declspec(dllimport) void* __stdcall GetModuleHandleW   (const wchar_t*);
    __declspec(dllimport) void* __stdcall LoadIconW          (void*, const wchar_t*);
    __declspec(dllimport) void* __stdcall LoadCursorW        (void*, const wchar_t*);
    __declspec(dllimport) u16   __stdcall RegisterClassExW   (WndClassExW*);
    __declspec(dllimport) i32   __stdcall AdjustWindowRect   (Rect*, u32, i32);
    __declspec(dllimport) i32   __stdcall GetSystemMetrics   (u32);
    __declspec(dllimport) void* __stdcall CreateWindowExW    (u32, const wchar_t*, const wchar_t*, u32, i32, i32, i32, i32, void*, void*, void*, void*);
    __declspec(dllimport) u32   __stdcall GetLastError       ();
    __declspec(dllimport) i32   __stdcall DestroyWindow      (void*);
    __declspec(dllimport) void  __stdcall ShowWindow         (void*, u32);
    __declspec(dllimport) void* __stdcall GetStockObject     (i32);
    __declspec(dllimport) i32   __stdcall SetWindowPos       (void*, void*, i32, i32, i32, i32, u32);
    __declspec(dllimport) i32   __stdcall GetWindowRect      (void*, Rect*);
    __declspec(dllimport) i32   __stdcall GetWindowPlacement (void*, WindowPlacement*);
    __declspec(dllimport) i32   __stdcall SetWindowPlacement (void*, const WindowPlacement*);
  }

  iptr GLWindow::message_proxy (void* handle, u32 message, uptr wp, iptr lp)
  {
    auto window = (GLWindow*) GetWindowLongPtrW (handle, gwlp_userdata);
    if (window)
      return window -> message_handler (message, wp, lp);
    else
      return DefWindowProcW (handle, message, wp, lp);
  }

  iptr GLWindow::message_handler (u32 message, uptr wp, iptr lp)
  {
    if (message == wm_size)
    {
      width  = (lp      ) & 0xffff;
      height = (lp >> 16) & 0xffff;
    }

    return handler (this, message, wp, lp);
  }

  void GLWindow::create (const wchar_t* title, Handler new_handler, bool new_fullscreen, uint window_width, uint window_height, void* new_user)
  {
    handler = new_handler;
    user    = new_user;
    
    WndClassExW wc = {
      sizeof (wc),
      0,
      message_proxy,
      0, 0,
      GetModuleHandleW (0),
      LoadIconW   (0, (const wchar_t*) idi_application),
      LoadCursorW (0, (const wchar_t*) idc_arrow),
      GetStockObject (black_brush),
      0,
      L"Co::GLWindow",
      0
    };
    
    u16 atom = RegisterClassExW (&wc);
    if (!atom)
      throw Rk::WinError ("Error registering window class");
    
    // Save window border sizes
    borders = Rect (0, 0, 0, 0);
    AdjustWindowRect (&borders, windowed_style, false);
    borders.left = -borders.left;
    borders.top  = -borders.top;

    i32 screen_width  = GetSystemMetrics (sm_cxscreen),
        screen_height = GetSystemMetrics (sm_cyscreen);
    
    u32 style_ex   = fullscreen_style_ex,
        style      = fullscreen_style,
        new_x      = 0,
        new_y      = 0,
        new_width  = screen_width,
        new_height = screen_height;

    if (!new_fullscreen)
    {
      style_ex   = windowed_style_ex;
      style      = windowed_style;
      new_x      = (screen_width  / 2) - (window_width  / 2) - borders.left;
      new_y      = (screen_height / 2) - (window_height / 2) - borders.top;
      new_width  = window_width  + borders.right + borders.left;
      new_height = window_height + borders.bottom + borders.top;
    }

    handle = CreateWindowExW (
      style_ex,
      (const wchar_t*) uptr (atom),
      title,
      style,
      new_x,
      new_y,
      new_width,
      new_height,
      0, 0,
      GetModuleHandleW (0),
      0
    );

    if (!handle)
      throw Rk::WinError ("Error creating window");
    
    SetWindowLongPtrW (handle, gwlp_userdata, this);

    fullscreen = false;
    if (new_fullscreen)
      set_fullscreen (true);
  }

  iptr GLWindow::message_default (u32 message, uptr wp, iptr lp)
  {
    return DefWindowProcW (handle, message, wp, lp);
  }

  void GLWindow::show (bool value)
  {
    if (value)
      ShowWindow (handle, sw_show);
    else
      ShowWindow (handle, sw_hide);
  }

  void GLWindow::set_fullscreen (bool new_fullscreen)
  {
    if (new_fullscreen == fullscreen)
      return;

    fullscreen = new_fullscreen;

    if (fullscreen)
    {
      auto placement = WindowPlacement ();
      placement.size = sizeof (placement);
      GetWindowPlacement (handle, &placement);

      win_rect  = placement.rect;
      win_maxed = (placement.show == sw_maximize);

      SetWindowLongW (handle, gwl_exstyle, fullscreen_style_ex);
      SetWindowLongW (handle, gwl_style,   fullscreen_style   );

      i32 screen_width  = GetSystemMetrics (sm_cxscreen),
          screen_height = GetSystemMetrics (sm_cyscreen);

      SetWindowPos (
        handle,
        0,
        0, 0,
        screen_width,
        screen_height,
        0x100 | 0x040 | 0x020 | 0x008
      );
    }
    else
    {
      SetWindowLongW (handle, gwl_exstyle, windowed_style_ex);
      SetWindowLongW (handle, gwl_style,   windowed_style   );

      auto placement = WindowPlacement ();
      placement.size = sizeof (placement);
      placement.rect = win_rect;
      placement.show = win_maxed ? sw_maximize : sw_show;
      SetWindowPlacement (handle, &placement);
      
      SetWindowPos (handle, 0, 0, 0, 0, 0, 0x100 | 0x040 | 0x020 | 0x002 | 0x001);
    }
  }

  GLWindow::~GLWindow ()
  {
    DestroyWindow (handle);
    handle = 0;
  }

}
