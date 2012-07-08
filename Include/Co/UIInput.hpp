//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_UIINPUT
#define CO_H_UIINPUT

#include <Rk/Types.hpp>

namespace Co
{
  enum MouseButton :
    u32
  {
    mouse_button_invalid = 0,

    mouse_button_left    = 1,
    mouse_button_right   = 2,
    mouse_button_middle  = 3,
    mouse_button_4       = 4,
    mouse_button_5       = 5,
    mouse_button_6       = 6
  };
  
  namespace Keys
  {
    enum Key :
      u32
    {
      key_invalid = 0,

      key_a,
      key_b,
      key_c,
      key_d,
      key_e,
      key_f,
      key_g,
      key_h,
      key_i,
      key_j,
      key_k,
      key_l,
      key_m,
      key_n,
      key_o,
      key_p,
      key_q,
      key_r,
      key_s,
      key_t,
      key_u,
      key_v,
      key_w,
      key_x,
      key_y,
      key_z,

      key_0,
      key_1,
      key_2,
      key_3,
      key_4,
      key_5,
      key_6,
      key_7,
      key_8,
      key_9,
      
      key_backtick,
      key_dash,
      key_equals,
      key_backspace,
      key_tab,
      key_left_square,
      key_right_square,
      key_return,
      key_capslock,
      key_semicolon,
      key_apostrophe,
      key_hash,
      key_left_shift,
      key_backslash,
      key_comma,
      key_period,
      key_slash,
      key_right_shift,
      key_left_control,
      key_left_alt,
      key_spacebar,
      key_right_alt,
      key_right_control,

      key_escape,
      key_f1,
      key_f2,
      key_f3,
      key_f4,
      key_f5,
      key_f6,
      key_f7,
      key_f8,
      key_f9,
      key_f10,
      key_f11,
      key_f12,
      key_f13,
      key_f14,
      key_f15,
      key_f16,
      key_f17,
      key_f18,
      key_f19,
      key_f20,
      key_f21,
      key_f22,
      key_f23,
      key_f24,

      key_print_screen,
      key_scroll_lock,
      key_pause,

      key_insert,
      key_delete,
      key_home,
      key_end,
      key_page_up,
      key_page_down,

      key_up,
      key_down,
      key_left,
      key_right,

      key_num_lock,
      key_pad_divide,
      key_pad_multiply,
      key_pad_minus,
      key_pad_plus,
      key_pad_point,

      key_pad_0,
      key_pad_1,
      key_pad_2,
      key_pad_3,
      key_pad_4,
      key_pad_5,
      key_pad_6,
      key_pad_7,
      key_pad_8,
      key_pad_9,

      key_count
    };

  }

  using namespace Keys;

  struct KeyState
  {
    u8 down    : 1,
       changed : 1,
       set     : 1;

    KeyState () :
      down    (false),
      changed (false),
      set     (false)
    { }
    
    bool pressed  () const { return down && changed; }
    bool released () const { return !down && changed; }

  };

  enum UIEventType :
    u32
  {
    ui_event_invalid     = 0,
    ui_event_mouse_down  = 1,
    ui_event_mouse_up    = 2,
    ui_event_mouse_wheel = 3,
    ui_event_mouse_move  = 4,
    ui_event_key_down    = 5,
    ui_event_key_up      = 6,
    ui_event_character   = 7
  };
  
  class UIEvent
  {
  public:
    UIEvent (UIEventType new_type = ui_event_invalid, i32 new_a = 0, i32 new_b = 0, i32 new_c = 0) :
      type (new_type),
      a    (new_a),
      b    (new_b),
      c    (new_c)
    { }
    
    UIEventType type;
    i32         a, b, c;

  };

  static inline UIEvent ui_mouse_down  (MouseButton button) { return UIEvent (ui_event_mouse_down, button); }
  static inline UIEvent ui_mouse_up    (MouseButton button) { return UIEvent (ui_event_mouse_up, button); }
  static inline UIEvent ui_mouse_wheel (i32 delta)          { return UIEvent (ui_event_mouse_wheel, delta); }
  static inline UIEvent ui_mouse_move  (i32 x, i32 y)       { return UIEvent (ui_event_mouse_move, x, y); }
  static inline UIEvent ui_key_down    (Key key)            { return UIEvent (ui_event_key_down, key); }
  static inline UIEvent ui_key_up      (Key key)            { return UIEvent (ui_event_key_up, key); }
  static inline UIEvent ui_character   (char32 codepoint)   { return UIEvent (ui_event_character, i32 (codepoint)); }
  
}

#endif
