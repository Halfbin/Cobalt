//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "GLWindow.hpp"

// Uses
#include <Rk/Exception.hpp>

namespace
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

  struct Rect
  {
    i32 left, top, right, bottom;
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
    
    idi_application = 32512,
    idc_arrow = 32512,
    
    gwlp_userdata = -21,

    sm_cxscreen = 0,
    sm_cyscreen = 1,
    
    sw_hide = 0,
    sw_show = 5,
    
    wm_size = 5,

    windowed_style      = ws_caption | ws_thickframe | ws_sysmenu | ws_minimizebox | ws_maximizebox,
    windowed_style_ex   = 0,
    fullscreen_style    = ws_popup,
    fullscreen_style_ex = ws_ex_topmost
  };

} // namespace

extern "C"
{
#ifdef _WIN64
  __declspec(dllimport) void* __stdcall GetWindowLongPtrW (void*, i32);
  __declspec(dllimport) void* __stdcall SetWindowLongPtrW (void*, i32, void*);
#else
  __declspec(dllimport) void* __stdcall GetWindowLongW (void*, i32);
  __declspec(dllimport) void* __stdcall SetWindowLongW (void*, i32, void*);
  #define GetWindowLongPtrW GetWindowLongW
  #define SetWindowLongPtrW SetWindowLongW
#endif
  __declspec(dllimport) iptr  __stdcall DefWindowProcW    (void*, u32, uptr, iptr);
  __declspec(dllimport) void* __stdcall GetModuleHandleW  (const wchar_t*);
  __declspec(dllimport) void* __stdcall LoadIconW         (void*, const wchar_t*);
  __declspec(dllimport) void* __stdcall LoadCursorW       (void*, const wchar_t*);
  __declspec(dllimport) u16   __stdcall RegisterClassExW  (WndClassExW*);
  __declspec(dllimport) i32   __stdcall AdjustWindowRect  (Rect*, u32, i32);
  __declspec(dllimport) i32   __stdcall GetSystemMetrics  (u32);
  __declspec(dllimport) void* __stdcall CreateWindowExW   (u32, const wchar_t*, const wchar_t*, u32, i32, i32, i32, i32, void*, void*, void*, void*);
  __declspec(dllimport) u32   __stdcall GetLastError      ();
  __declspec(dllimport) i32   __stdcall DestroyWindow     (void*);
  __declspec(dllimport) void  __stdcall ShowWindow        (void*, u32);
}

namespace Co
{
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

  void GLWindow::create (const wchar_t* title, Handler new_handler, bool fullscreen, uint window_width, uint window_height, void* new_user)
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
      0, 0,
      L"Co::GLWindow",
      0
    };
    
    u16 atom = RegisterClassExW (&wc);
    if (!atom)
      throw Rk::Exception ("Error registering window class");
    
    // Save window border sizes
    Rect test_rect = { 0, 0, 0, 0 };
    AdjustWindowRect (&test_rect, windowed_style, false);
    uint top_border    = -test_rect.top,
         left_border   = -test_rect.left,
         right_border  =  test_rect.right,
         bottom_border =  test_rect.bottom;
    
    u32 style    = fullscreen ? fullscreen_style    : windowed_style,
        style_ex = fullscreen ? fullscreen_style_ex : windowed_style_ex;
    
    u32 sw = GetSystemMetrics (sm_cxscreen),
        sh = GetSystemMetrics (sm_cyscreen);
    
    u32 x = fullscreen ? 0 : (sw / 2) - (window_width  / 2),
        y = fullscreen ? 0 : (sh / 2) - (window_height / 2),
        w = fullscreen ? sw : window_width + left_border + right_border,
        h = fullscreen ? sh : window_height + bottom_border + top_border;
    
    handle = CreateWindowExW (
      style_ex,
      (const wchar_t*) uptr (atom),
      title,
      style | ws_clipsiblings,
      x, y, w, h,
      0, 0,
      GetModuleHandleW (0),
      0
    );

    if (!handle)
    {
      throw Rk::Exception ("Error creating window")
        << " GetLastError () " << GetLastError ();
    }
    
    SetWindowLongPtrW (handle, gwlp_userdata, this);
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

  GLWindow::~GLWindow ()
  {
    DestroyWindow (handle);
    handle = 0;
  }

}
