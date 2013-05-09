#ifndef LOVE_WINDOW_PPAPI_INPUT_H
#define LOVE_WINDOW_PPAPI_INPUT_H

#include <ppapi/c/pp_input_event.h>
#include <ppapi/cpp/input_event.h>
#include <vector>

namespace love {
namespace window {
namespace ppapi {

enum InputType {
  INPUT_NONE,
  INPUT_KEY,
  INPUT_CHARACTER,
  INPUT_MOUSE,
  INPUT_WHEEL,
  INPUT_FOCUS,
  INPUT_TYPE_MAX,
};

enum MouseButton {
  MOUSE_NONE,
  MOUSE_LEFT,
  MOUSE_MIDDLE,
  MOUSE_RIGHT,
  MOUSE_BUTTON_MAX,
};

enum KeyType {
  RAW_KEY_DOWN,
  KEY_DOWN,
  KEY_UP,
  KEY_TYPE_MAX,
};

enum MouseType {
  MOUSE_DOWN,
  MOUSE_UP,
  MOUSE_MOVE,
  MOUSE_ENTER,
  MOUSE_LEAVE,
  MOUSE_TYPE_MAX,
};

enum KeyCode {
  KEY_CODE_LBUTTON = 0x01,
  KEY_CODE_RBUTTON = 0x02,
  KEY_CODE_CANCEL = 0x03,
  KEY_CODE_MBUTTON = 0x04,
  KEY_CODE_XBUTTON1 = 0x05,
  KEY_CODE_XBUTTON2 = 0x06,
  KEY_CODE_BACK = 0x08,  // Backspace
  KEY_CODE_TAB = 0x09,
  KEY_CODE_CLEAR = 0x0c,
  KEY_CODE_RETURN = 0x0d,  // Enter
  KEY_CODE_SHIFT = 0x10,
  KEY_CODE_CONTROL = 0x11,  // Ctrl
  KEY_CODE_MENU = 0x12,  // Alt
  KEY_CODE_PAUSE = 0x13,
  KEY_CODE_CAPITAL = 0x14,  // CapsLk
  KEY_CODE_ESCAPE = 0x1b,
  KEY_CODE_SPACE = 0x20,
  KEY_CODE_PRIOR = 0x21,  // PgUp
  KEY_CODE_NEXT = 0x22,  // PgDn
  KEY_CODE_END = 0x23,
  KEY_CODE_HOME = 0x24,
  KEY_CODE_LEFT = 0x25,
  KEY_CODE_UP = 0x26,
  KEY_CODE_RIGHT = 0x27,
  KEY_CODE_DOWN = 0x28,
  KEY_CODE_SELECT = 0x29,
  KEY_CODE_PRINT = 0x2a,
  KEY_CODE_EXECUTE = 0x2b,
  KEY_CODE_SNAPSHOT = 0x2c,  // PrtSc
  KEY_CODE_INSERT = 0x2d,
  KEY_CODE_DELETE = 0x2e,
  KEY_CODE_HELP = 0x2f,
  KEY_CODE_0 = 0x30,
  KEY_CODE_1 = 0x31,
  KEY_CODE_2 = 0x32,
  KEY_CODE_3 = 0x33,
  KEY_CODE_4 = 0x34,
  KEY_CODE_5 = 0x35,
  KEY_CODE_6 = 0x36,
  KEY_CODE_7 = 0x37,
  KEY_CODE_8 = 0x38,
  KEY_CODE_9 = 0x39,
  KEY_CODE_A = 0x41,
  KEY_CODE_B = 0x42,
  KEY_CODE_C = 0x43,
  KEY_CODE_D = 0x44,
  KEY_CODE_E = 0x45,
  KEY_CODE_F = 0x46,
  KEY_CODE_G = 0x47,
  KEY_CODE_H = 0x48,
  KEY_CODE_I = 0x49,
  KEY_CODE_J = 0x4a,
  KEY_CODE_K = 0x4b,
  KEY_CODE_L = 0x4c,
  KEY_CODE_M = 0x4d,
  KEY_CODE_N = 0x4e,
  KEY_CODE_O = 0x4f,
  KEY_CODE_P = 0x50,
  KEY_CODE_Q = 0x51,
  KEY_CODE_R = 0x52,
  KEY_CODE_S = 0x53,
  KEY_CODE_T = 0x54,
  KEY_CODE_U = 0x55,
  KEY_CODE_V = 0x56,
  KEY_CODE_W = 0x57,
  KEY_CODE_X = 0x58,
  KEY_CODE_Y = 0x59,
  KEY_CODE_Z = 0x5a,
  KEY_CODE_LWIN = 0x5b,
  KEY_CODE_RWIN = 0x5c,
  KEY_CODE_APPS = 0x5d,
  KEY_CODE_SLEEP = 0x5e,
  KEY_CODE_NUMPAD0 = 0x60,
  KEY_CODE_NUMPAD1 = 0x61,
  KEY_CODE_NUMPAD2 = 0x62,
  KEY_CODE_NUMPAD3 = 0x63,
  KEY_CODE_NUMPAD4 = 0x64,
  KEY_CODE_NUMPAD5 = 0x65,
  KEY_CODE_NUMPAD6 = 0x66,
  KEY_CODE_NUMPAD7 = 0x67,
  KEY_CODE_NUMPAD8 = 0x68,
  KEY_CODE_NUMPAD9 = 0x69,
  KEY_CODE_MULTIPLY = 0x6a,
  KEY_CODE_ADD = 0x6b,
  KEY_CODE_SEPARATOR = 0x6c,
  KEY_CODE_SUBTRACT = 0x6d,
  KEY_CODE_DECIMAL = 0x6e,
  KEY_CODE_DIVIDE = 0x6f,
  KEY_CODE_F1 = 0x70,
  KEY_CODE_F2 = 0x71,
  KEY_CODE_F3 = 0x72,
  KEY_CODE_F4 = 0x73,
  KEY_CODE_F5 = 0x74,
  KEY_CODE_F6 = 0x75,
  KEY_CODE_F7 = 0x76,
  KEY_CODE_F8 = 0x77,
  KEY_CODE_F9 = 0x78,
  KEY_CODE_F10 = 0x79,
  KEY_CODE_F11 = 0x7a,
  KEY_CODE_F12 = 0x7b,
  KEY_CODE_F13 = 0x7c,
  KEY_CODE_F14 = 0x7d,
  KEY_CODE_F15 = 0x7e,
  KEY_CODE_F16 = 0x7f,
  KEY_CODE_F17 = 0x80,
  KEY_CODE_F18 = 0x81,
  KEY_CODE_F19 = 0x82,
  KEY_CODE_F20 = 0x83,
  KEY_CODE_F21 = 0x84,
  KEY_CODE_F22 = 0x85,
  KEY_CODE_F23 = 0x86,
  KEY_CODE_F24 = 0x87,
  KEY_CODE_NUMLOCK = 0x90,
  KEY_CODE_SCROLL = 0x91,
  KEY_CODE_LSHIFT = 0xa0,
  KEY_CODE_RSHIFT = 0xa1,
  KEY_CODE_LCONTROL = 0xa2,
  KEY_CODE_RCONTROL = 0xa3,
  KEY_CODE_LMENU = 0xa4,
  KEY_CODE_RMENU = 0xa5,
  KEY_CODE_BROWSER_BACK = 0xa6,
  KEY_CODE_BROWSER_FORWARD = 0xa7,
  KEY_CODE_BROWSER_REFRESH = 0xa8,
  KEY_CODE_BROWSER_STOP = 0xa9,
  KEY_CODE_BROWSER_SEARCH = 0xaa,
  KEY_CODE_BROWSER_FAVORITES = 0xab,
  KEY_CODE_BROWSER_HOME = 0xac,
  KEY_CODE_VOLUME_MUTE = 0xad,
  KEY_CODE_VOLUME_DOWN = 0xae,
  KEY_CODE_VOLUME_UP = 0xaf,
  KEY_CODE_MEDIA_NEXT_TRACK = 0xb0,
  KEY_CODE_MEDIA_PREV_TRACK = 0xb1,
  KEY_CODE_MEDIA_STOP = 0xb2,
  KEY_CODE_MEDIA_PLAY_PAUSE = 0xb3,
  KEY_CODE_LAUNCH_MAIL = 0xb4,
  KEY_CODE_LAUNCH_MEDIA_SELECT = 0xb5,
  KEY_CODE_LAUNCH_APP1 = 0xb6,
  KEY_CODE_LAUNCH_APP2 = 0xb7,
  KEY_CODE_OEM_1 = 0xba,
  KEY_CODE_OEM_PLUS = 0xbb,
  KEY_CODE_OEM_COMMA = 0xbc,
  KEY_CODE_OEM_MINUS = 0xbd,
  KEY_CODE_OEM_PERIOD = 0xbe,
  KEY_CODE_OEM_2 = 0xbf,
  KEY_CODE_OEM_3 = 0xc0,
  KEY_CODE_OEM_4 = 0xdb,
  KEY_CODE_OEM_5 = 0xdc,
  KEY_CODE_OEM_6 = 0xdd,
  KEY_CODE_OEM_7 = 0xde,
  KEY_CODE_OEM_8 = 0xdf,
  KEY_CODE_OEM_102 = 0xe2,
  KEY_CODE_PROCESSKEY = 0xe2,
  KEY_CODE_ATTN = 0xf6,
  KEY_CODE_CRSEL = 0xf7,
  KEY_CODE_EXSEL = 0xf8,
  KEY_CODE_EREOF = 0xf9,
  KEY_CODE_PLAY = 0xfa,
  KEY_CODE_ZOOM = 0xfb,
  KEY_CODE_PA1 = 0xfd,
  KEY_CODE_OEM_CLEAR = 0xfe,
  KEY_CODE_MAX = 0xff,
};

struct KeyEvent {
  KeyType type;
  uint32_t code;
  char text[5];
};

struct CharacterEvent {
  char text[5];
};

struct MouseEvent {
  MouseType type;
  MouseButton button;
  int32_t x;
  int32_t y;
  int32_t movement_x;
  int32_t movement_y;
};

struct WheelEvent {
  float delta_x;
  float delta_y;
  float ticks_x;
  float ticks_y;
  bool scroll_by_page;
};

struct FocusEvent {
  bool has_focus;
};

struct InputEvent {
  InputType type;
  uint32_t modifiers;
  union {
    KeyEvent key;
    CharacterEvent character;
    MouseEvent mouse;
    WheelEvent wheel;
    FocusEvent focus;
  };
};

typedef std::vector<InputEvent> InputEvents;

void InitializeEventQueue();
void EnqueueFocusEvent(bool has_focus);
void EnqueueEvent(const pp::InputEvent& event);
void EnqueueEvent(const InputEvent& event);
bool DequeueEvent(InputEvent* out_event);
void DequeueAllEvents(InputEvents* out_events);
void WaitForEvent();

// Mouse state
int GetMouseX();
int GetMouseY();
bool IsMouseButtonPressed(MouseButton);

// Keyboard state
void SetKeyRepeat(bool repeat);
bool IsKeyPressed(uint32_t code);

}  // ppapi
}  // window
}  // love

#endif  // LOVE_WINDOW_PPAPI_INPUT_H
