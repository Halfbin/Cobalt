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
}

enum Messages
{
  wm_size        = 0x0005,
  wm_close       = 0x0010,
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

  iptr Client::handler (u32 message, uptr wp, iptr lp)
  {
    switch (message)
    {
      case wm_close:
        engine -> stop ();
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
        }
        // Game control input
        else
        {

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

  Client::Client (Rk::StringRef config_path) :
    Host (clock, Co::log)
  {
    ui_enabled = true;

    init_key_tab ();

    Rk::StringRef game_name = "Iridium";

    // Parse configuration
    Rk::StringOutStream game_path;
    game_path << "../" << game_name << "/";

    // Load subsystem modules
    get_object (engine);
    get_object (queue);
    get_object (renderer);
    get_object (filesystem);

    auto game = Rk::Module (game_path.string () + "Binaries/Co-Game" CO_SUFFIX ".dll").create <Game> ();

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
    engine.reset   ();
    renderer.reset ();

    // Spin down worker threads
    queue -> stop ();
    pool.clear ();
  }
  
  //
  // run
  //
  void Client::run ()
  {
    window.show ();

    float next_update = engine -> start ();
    
    for (;;)
    {
      Message msg;
      while (PeekMessageW (&msg, 0, 0, 0, 1))
      {
        DispatchMessageW (&msg);
        if (clock.time () + 0.01 >= next_update)
          break;
      }
      
      engine -> wait ();

      bool running = engine -> update (
        next_update,
        window.get_width  (),
        window.get_height (),
        ui_events.data (),
        ui_events.size ()
      );
      if (!running)
        break;

      ui_events.clear ();
    }
  }
  
  typedef std::map <std::string, std::string> ModuleConfig;
  
  template <typename Interface>
  ModuleConfig::value_type implements (Rk::StringRef name)
  {
    return std::make_pair (Interface::ix_name ().string (), name.string ());
  }

  ModuleConfig::value_type cfg [] =
  {
    implements <Engine>         ("Co-Engine"),
    implements <WorkQueue>      ("Co-WorkQueue"),
    implements <Renderer>       ("Co-GLRenderer"),
    implements <TextureFactory> ("Co-Texture"),
    implements <ModelFactory>   ("Co-Model"),
    implements <FontFactory>    ("Co-Font"),
    implements <Filesystem>     ("Co-Filesystem")
  };

  ModuleConfig module_config (std::begin (cfg), std::end (cfg));
  
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
    if (ui_enabled && !new_enabled)
    {
      ShowCursor (false);
    }
    else if (!ui_enabled && new_enabled)
    {
      ShowCursor (true);
    }

    ui_enabled = new_enabled;
  }

} // namespace Co
