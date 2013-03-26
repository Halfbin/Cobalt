//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/ClientFrontend.hpp>

// Uses
#include <Co/Engine.hpp>

struct Point
{
  i32 x, y;
};

struct Message
{
  void* window;
  u32 message;
  uptr wp;
  iptr lp;
  u32 time;
  Point cursor;
};

extern "C"
{
  #define fromdll __declspec(dllimport)
  fromdll void  __stdcall PostQuitMessage (u32);
  fromdll i32   __stdcall PeekMessageW (Message*, void*, u32, u32, u32);
  fromdll i32   __stdcall DispatchMessageW (const Message*);
  fromdll i32   __stdcall ShowCursor (i32);
  fromdll i32   __stdcall GetKeyboardState (u8*);
  fromdll i32   __stdcall GetSystemMetrics (i32);
  fromdll i32   __stdcall GetCursorPos (Point*);
  fromdll i32   __stdcall SetCursorPos (i32, i32);
  fromdll void* __stdcall GetCurrentThread ();
  fromdll i32   __stdcall SetThreadPriority (void*, i32);
}

enum Messages
{
  wm_size          = 0x0005,
  wm_close         = 0x0010,
  wm_activateapp   = 0x001c,
  wm_displaychange = 0x007e,
  wm_keydown       = 0x0100,
  wm_keyup         = 0x0101,
  wm_char          = 0x0103,
  wm_syskeydown    = 0x0104,
  wm_mousemove     = 0x0200,
  wm_lbuttondown   = 0x0201,
  wm_lbuttonup     = 0x0202,
  wm_rbuttondown   = 0x0204,
  wm_rbuttonup     = 0x0205,
  wm_mbuttondown   = 0x0207,
  wm_mbuttonup     = 0x0208
};

enum
{
  sm_cxscreen = 0,
  sm_cyscreen = 1,
};

namespace Co
{
  static Key keytab [256] = { key_invalid };

  static void init_key_tab ()
  {
    keytab [0x08] = key_backspace;
    keytab [0x09] = key_tab;
    keytab [0x0d] = key_return;
    keytab [0x13] = key_pause;
    keytab [0x14] = key_capslock;
    keytab [0x1b] = key_escape;
    keytab [0x20] = key_spacebar;
    keytab [0x21] = key_page_up;
    keytab [0x22] = key_page_down;
    keytab [0x23] = key_end;
    keytab [0x24] = key_home;
    keytab [0x25] = key_left;
    keytab [0x26] = key_up;
    keytab [0x27] = key_right;
    keytab [0x28] = key_down;
    keytab [0x2c] = key_print_screen;
    keytab [0x2d] = key_insert;
    keytab [0x2e] = key_delete;
    
    for (uint i = 0; i != 10; i++)
      keytab [0x30 + i] = Key (key_0 + i);

    for (uint i = 0; i != 26; i++)
      keytab [0x41 + i] = Key (key_a + i);

    for (uint i = 0; i != 10; i++)
      keytab [0x60 + i] = Key (key_pad_0 + i);

    keytab [0x6a] = key_pad_multiply;
    keytab [0x6b] = key_pad_plus;
    keytab [0x6d] = key_pad_minus;
    keytab [0x6e] = key_pad_point;
    keytab [0x6f] = key_pad_divide;

    for (uint i = 0; i != 24; i++)
      keytab [0x70 + i] = Key (key_f1 + i);

    keytab [0x90] = key_num_lock;
    keytab [0x91] = key_scroll_lock;

    keytab [0xa0] = key_left_shift;
    keytab [0xa1] = key_right_shift;
    keytab [0xa2] = key_left_control;
    keytab [0xa3] = key_right_control;

    keytab [0xba] = key_semicolon;
    keytab [0xbb] = key_equals;
    keytab [0xbc] = key_comma;
    keytab [0xbd] = key_dash;
    keytab [0xbe] = key_period;
    keytab [0xbf] = key_slash;
    keytab [0xc0] = key_backtick;
    keytab [0xdb] = key_left_square;
    keytab [0xdc] = key_backslash;
    keytab [0xdd] = key_right_square;
    keytab [0xde] = key_apostrophe;
    keytab [0xdf] = key_hash;
  }

  static Key translate_key (u32 message, uptr wp, iptr lp)
  {
    return keytab [wp & 0xff];
  }

  uint mid_x, mid_y;

  static void update_mids ()
  {
    mid_x = GetSystemMetrics (sm_cxscreen) / 2;
    mid_y = GetSystemMetrics (sm_cyscreen) / 2;
  }

  iptr ClientFrontend::handle_message (ClientWindow*, u32 message, uptr wp, iptr lp)
  {
    switch (message)
    {
      case wm_close:
        running = false;
      return 0;

      case wm_activateapp:
        if (!wp)
        {
          prev_ui_enabled = ui_enabled;
          enable_ui (true);
        }
        else
        {
          enable_ui (prev_ui_enabled);
        }
      return 0;

      case wm_displaychange:
        update_mids ();
      break;

      case wm_syskeydown:
        if (wp == 0x0d && !(lp & (1 << 30))) // Alt+Enter
        {
          window.set_fullscreen (!window.is_fullscreen ());
          return 0;
        }
      break;

      default:
        // UI input
        if (ui_enabled)
        {
          switch (message)
          {
            case wm_mousemove:
              ui_events.push_back (ui_mouse_move ((lp & 0xffff), ((lp >> 16) & 0xffff)));
            return 0;

            case wm_lbuttondown:
              ui_events.push_back (ui_mouse_down (mouse_button_left));
            return 0;

            case wm_lbuttonup:
              ui_events.push_back (ui_mouse_up (mouse_button_left));
            return 0;

            case wm_rbuttondown:
              ui_events.push_back (ui_mouse_down (mouse_button_right));
            return 0;

            case wm_rbuttonup:
              ui_events.push_back (ui_mouse_up (mouse_button_right));
            return 0;

            case wm_mbuttondown:
              ui_events.push_back (ui_mouse_down (mouse_button_middle));
            return 0;

            case wm_mbuttonup:
              ui_events.push_back (ui_mouse_up (mouse_button_middle));
            return 0;

            case wm_keydown:
              ui_events.push_back (ui_key_down (translate_key (message, wp, lp)));
            return 0;

            case wm_keyup:
              ui_events.push_back (ui_key_up (translate_key (message, wp, lp)));
            return 0;

            case wm_char:
            {
              char16 cp (wp & 0xffff);

              // Are the top five bits 11011?
              if ((cp & 0xf800) == 0xd800) // Surrogate - Ignore for now
              {
                
              }
              else // Straight BMP codepoint
              {
                ui_events.push_back (ui_character (cp));
              }
            }
            return 0;

            default:;
          };

          break;
        }
        // Game control input
        else
        {
          // nothing atm
          break;
        }
    }   

    return window.message_default (message, wp, lp);
  }

  // TODO
  void ClientFrontend::worker (RenderDevice& device, Filesystem& fs)
  {
    log () << "- Worker thread starting\n";

    SetThreadPriority (GetCurrentThread (), -1);

    auto rc = device.create_context ();
    
    for (;;)
    {
      try
      {
        queue -> work (*rc, fs);
        break;
      }
      catch (...)
      {
        log_exception (log, "in worker thread");
      }
    }

    log () << "- Worker thread exiting\n";
  }

  void ClientFrontend::update_keyboard ()
  {
    u8 raw_keystate [256];
    GetKeyboardState (raw_keystate);

    for (uint vk = 0; vk != 256; vk++)
    {
      auto key = keytab [vk];
      if (key == key_invalid)
        continue;

      bool down = (raw_keystate [vk] >> 7) ? true : false;
      bool set  = (raw_keystate [vk] &  1) ? true : false;

      keyboard [key].changed = (down != keyboard [key].down);
      keyboard [key].down = down;
      keyboard [key].set = set;
    }
  }

  v2f ClientFrontend::update_mouse ()
  {
    if (ui_enabled)
      return v2f (0, 0);

    float scale = 1.0f / float (std::min (mid_x, mid_y));

    Point cursor;
    GetCursorPos (&cursor);
    SetCursorPos (mid_x, mid_y);

    return v2f (
      cursor.x - mid_x,
      cursor.y - mid_y
    ) * scale;
  }

  void ClientFrontend::run_game ()
  {
    clock.restart ();
    Engine eng (clock, 50.0f);

    client -> client_start ();

    while (running && !switch_game ())
    {
      // Message pump
      Message msg;
      while (PeekMessageW (&msg, 0, 0, 0, 1))
        DispatchMessageW (&msg);

      // Input
      update_keyboard ();
      auto mouse_delta = update_mouse ();
      client -> client_input (ui_events.data (), ui_events.size (), keyboard, mouse_delta);
      ui_events.clear ();

      // Worker Sync
      queue -> do_completions ();

      // Update
      eng.update_clock ();

      while (eng.tick ())
        client -> client_tick (eng.time (), eng.time_step ());

      // Render
      renderer -> width  = window.get_width ();
      renderer -> height = window.get_height ();
      client -> render (*renderer, eng.alpha ());
      renderer -> render_frame ();
    }

    client -> client_stop ();
  }

  void ClientFrontend::run ()
  {
    window.show ();
    running = true;
    
    while (running)
    {
      client = next_client;
      next_client = nullptr;

      server = next_server;
      next_server = nullptr;

      run_game ();
    }
  }

  ClientFrontend::ClientFrontend (Rk::StringRef new_exe_name, Rk::WStringRef app_name, bool fullscreen, uint w, uint h) :
    exe_name (new_exe_name.string () + CO_SUFFIX),
    log_file (exe_name, false),
    log_str  (log_file.stream ()),
    log      (log_str),
    modman   (log),
    window   (app_name, Rk::method_proxy (this, &ClientFrontend::handle_message), fullscreen, w, h)
  {
    try
    {
      init_key_tab ();
      ui_enabled = true;
      prev_ui_enabled = true;

      update_mids ();

      log () << "* " << exe_name << " starting\n";
      
      queue = load_module <WorkQueueRoot> ("Co-WorkQueue") -> create_queue (log);

      filesystem = load_module <FilesystemRoot> ("Co-Filesystem") -> create_fs (log);

      renderer = load_module <RendererRoot> ("Co-GLRenderer") -> create_renderer (log, *queue, clock);
      renderer -> init (window.get_handle ());

      audio = load_module <AudioRoot> ("Co-Audio-XA2") -> create_audio_service (log, *queue);

      // Start worker threads
      auto thread_renderer   = renderer;
      auto thread_filesystem = filesystem;
      pool.reset (
        4, 
        [this, thread_renderer, thread_filesystem]
        {
          worker (*thread_renderer, *thread_filesystem);
        }
      );
    }
    catch (...)
    {
      log_exception (log, "during frontend initialization");
      throw std::runtime_error ("(Proxy exception)");
    }
  }
  
  void ClientFrontend::enable_ui (bool new_enabled)
  {
    if (ui_enabled != new_enabled)
      ShowCursor (new_enabled);

    ui_enabled = new_enabled;
  }

  ClientFrontend::~ClientFrontend ()
  {
    server.reset ();
    client.reset ();
    next_server.reset ();
    next_client.reset ();
    queue -> stop ();
    log () << "* " << exe_name << " exiting\n";
  }

}
