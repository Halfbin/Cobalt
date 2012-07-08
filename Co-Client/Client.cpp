//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include "Client.hpp"

// Uses
#include <Co/Texture.hpp>
#include <Co/Model.hpp>
#include <Co/Font.hpp>

#include <Co/RenderDevice.hpp>
#include <Co/Renderer.hpp>
#include <Co/Engine.hpp>
#include <Co/Game.hpp>

#include <Rk/ShortString.hpp>
#include <Rk/Module.hpp>
#include <Rk/Guard.hpp>

#include "GLWindow.hpp"
#include "Common.hpp"

struct Point
{
  i32 x, y;
};

struct Message
{
  void* window;
  u32   message;
  uptr  wp;
  iptr  lp;
  u32   time;
  Point cursor;
};

extern "C"
{
  __declspec(dllimport) void __stdcall PostQuitMessage  (u32);
  __declspec(dllimport) i32  __stdcall PeekMessageW     (Message*, void*, u32, u32, u32);
  __declspec(dllimport) i32  __stdcall DispatchMessageW (const Message*);
  __declspec(dllimport) i32  __stdcall ShowCursor       (i32);
  __declspec(dllimport) i32  __stdcall GetKeyboardState (u8*);
  __declspec(dllimport) i32  __stdcall GetSystemMetrics (i32);
  __declspec(dllimport) i32  __stdcall GetCursorPos     (Point*);
  __declspec(dllimport) i32  __stdcall SetCursorPos     (i32, i32);
}

enum Messages
{
  wm_size        = 0x0005,
  wm_close       = 0x0010,
  wm_activateapp = 0x001c,
  wm_keydown     = 0x0100,
  wm_keyup       = 0x0101,
  wm_char        = 0x0103,
  wm_syskeydown  = 0x0104,
  wm_mousemove   = 0x0200,
  wm_lbuttondown = 0x0201,
  wm_lbuttonup   = 0x0202,
  wm_rbuttondown = 0x0204,
  wm_rbuttonup   = 0x0205,
  wm_mbuttondown = 0x0207,
  wm_mbuttonup   = 0x0208
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

  iptr Client::handler_proxy (GLWindow* win, u32 message, uptr wp, iptr lp)
  {
    auto client = (Client*) win -> get_user ();
    return client -> handler (message, wp, lp);
  }

  bool prev_ui_enabled;

  iptr Client::handler (u32 message, uptr wp, iptr lp)
  {
    switch (message)
    {
      case wm_close:
        engine -> stop ();
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

              // Are the top five bits are 11011?
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
  
  void Client::worker (RenderDevice& device, Filesystem& fs)
  {
    log () << "- Worker thread starting\n";
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

  template <typename Interface>
  std::pair <std::string, std::string> mod (std::string name)
  {
    return std::make_pair (Interface::ix_name ().string (), name);
  }

  std::pair <std::string, std::string> default_mods [] =
  {
    mod <Engine>         ("Co-Engine"),
    mod <WorkQueue>      ("Co-WorkQueue"),
    mod <Renderer>       ("Co-GLRenderer"),
    mod <TextureFactory> ("Co-Texture"),
    mod <ModelFactory>   ("Co-Model"),
    mod <FontFactory>    ("Co-Font"),
    mod <Filesystem>     ("Co-Filesystem")
  };

  static int mid_x, mid_y;

  Client::Client (Rk::StringRef config_path) :
    Host (clock, Co::log)
  {
    ui_enabled = true;

    mid_x = GetSystemMetrics (sm_cxscreen) / 2;
    mid_y = GetSystemMetrics (sm_cyscreen) / 2;

    init_key_tab ();

    // Parse configuration
    module_config.insert (std::begin (default_mods), std::end (default_mods));

    Rk::StringRef game_name = "SH";
    Rk::StringOutStream game_path;
    game_path << "../" << game_name << "/";

    module_config.insert (mod <Game> (game_path.string () + "Binaries/Co-Game"));

    // Load subsystem modules
    get_object (engine);
    get_object (queue);
    get_object (renderer);
    auto filesystem = get_object <Filesystem> ();
    auto game = get_object <Game> ();

    // Initialize subsystems
    Rk::StringWOutStream title;
    title << L"Cobalt Client"; 
    window.create (title.string ().c_str (), handler_proxy, false, 1280, 720, this);

    filesystem -> init (game_path.string ());

    renderer -> init (window.get_handle (), clock, log);
    
    // Start worker threads
    auto thread_renderer   = renderer;
    auto thread_filesystem = filesystem;
    pool.resize (4);
    for (auto thread = pool.begin (); thread != pool.end (); thread++)
    {
      thread -> execute (
        [this, thread_renderer, thread_filesystem]
        {
          worker (*thread_renderer, *thread_filesystem);
        }
      );
    }

    engine -> init (*this, renderer, *queue, game);
  }
  
  Client::~Client ()
  {
    window.hide ();

    // Specific shutdown order is important
    // Kill anything that can own resources
    objects.clear ();    // Release otherwise-unreferenced global objects
    engine.reset ();     // Destroy engine and game
    
    // Now that we're guaranteed to get no more jobs, finish any in the queue, and then
    // spin down the worker threads
    queue -> stop (); 
    pool.clear ();
  }

  //
  // Input
  //
  void Client::update_keyboard ()
  {
    u8 raw_keystate [256];
    GetKeyboardState (raw_keystate);

    for (uint vk = 0; vk != 256; vk++)
    {
      auto key = keytab [vk];
      if (key == key_invalid)
        continue;

      bool down = raw_keystate [vk] >> 7;
      bool set  = raw_keystate [vk] &  1;

      if (down && vk == 0x57)
        __asm nop;

      if (down != keyboard [key].down)
        keyboard [key].changed = true;
      else
        keyboard [key].changed = false;

      keyboard [key].down = down;
      keyboard [key].set  = set;
    }
  }

  v2f Client::update_mouse ()
  {
    if (ui_enabled)
      return v2f (0, 0);

    int min_mid = std::min (mid_x, mid_y);

    Point cursor;
    GetCursorPos (&cursor);
    SetCursorPos (mid_x, mid_y);

    return v2f ((cursor.x - mid_x) / float (min_mid), (cursor.y - mid_y) / float (min_mid));
  }

  //
  // run
  //
  void Client::run ()
  {
    window.show ();

    engine -> start ();
    
    for (;;)
    {
      // Message pump
      Message msg;
      while (PeekMessageW (&msg, 0, 0, 0, 1))
        DispatchMessageW (&msg);
      
      // Input
      update_keyboard ();
      auto mouse_delta = update_mouse ();

      // Update
      bool running = engine -> update (
        window.get_width  (),
        window.get_height (),
        ui_events.data (),
        ui_events.size (),
        keyboard,
        mouse_delta
      );

      if (!running)
        break;

      ui_events.clear ();
    }
  }
  
  std::shared_ptr <void> Client::get_object (Rk::StringRef type)
  {
    auto object_iter = objects.find (type.string ());
    if (object_iter != objects.end ())
      return object_iter -> second;

    auto name_iter = module_config.find (type.string ());
    if (name_iter != module_config.end ())
    {
      auto ptr = load_module (name_iter -> second).create (type);
      objects.insert (std::make_pair (type.string (), ptr));
      return ptr;
    }
    else
    {
      throw std::runtime_error ("No module provides objects of type \"" + type.string () + "\"");
    }
  }

  void Client::enable_ui (bool new_enabled)
  {
    if (ui_enabled != new_enabled)
      ShowCursor (new_enabled);

    ui_enabled = new_enabled;
  }

} // namespace Co
