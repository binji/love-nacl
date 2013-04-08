/**
 **/

#include <common/config.h>

#include "Keyboard.h"

namespace love {
namespace keyboard {
namespace ppapi {

const char * Keyboard::getName() const {
  return "love.keyboard.ppapi";
}

bool Keyboard::isDown(Key * keylist) const {
  using namespace love::window::ppapi;
  KeyCode key_code;
  for (Key key = *keylist; key != KEY_MAX_ENUM; key = *(++keylist)) {
    if (keys.find(key, key_code) && IsKeyPressed(key_code))
      return true;
  }

  return false;
}

void Keyboard::setKeyRepeat(int delay, int interval) const {
}

int Keyboard::getKeyRepeatDelay() const {
  return 0;
}

int Keyboard::getKeyRepeatInterval() const {
  return 0;
}

bool Keyboard::Convert(LoveKey in_key, PPAPIKey* out_key) {
  return keys.find(in_key, *out_key);
}

bool Keyboard::Convert(PPAPIKey in_key, LoveKey* out_key) {
  return keys.find(in_key, *out_key);
}

Keyboard::KeyEnumMap::Entry Keyboard::keyEntries[] = {
  { Keyboard::KEY_BACKSPACE, love::window::ppapi::KEY_CODE_BACK },
  { Keyboard::KEY_TAB, love::window::ppapi::KEY_CODE_TAB},
  { Keyboard::KEY_CLEAR, love::window::ppapi::KEY_CODE_CLEAR},
  { Keyboard::KEY_RETURN, love::window::ppapi::KEY_CODE_RETURN},
  { Keyboard::KEY_PAUSE, love::window::ppapi::KEY_CODE_PAUSE},
  { Keyboard::KEY_ESCAPE, love::window::ppapi::KEY_CODE_ESCAPE },
  { Keyboard::KEY_SPACE, love::window::ppapi::KEY_CODE_SPACE },
//  { Keyboard::KEY_EXCLAIM, love::window::ppapi::KEY_CODE_1 },
//  { Keyboard::KEY_QUOTEDBL, love::window::ppapi::KEY_CODE_OEM_7 },
//  { Keyboard::KEY_HASH, love::window::ppapi::KEY_CODE_3 },
//  { Keyboard::KEY_DOLLAR, love::window::ppapi::KEY_CODE_4 },
//  { Keyboard::KEY_AMPERSAND, love::window::ppapi::KEY_CODE_7 },
  { Keyboard::KEY_QUOTE, love::window::ppapi::KEY_CODE_OEM_7 },
//  { Keyboard::KEY_LEFTPAREN, love::window::ppapi::KEY_CODE_9 },
//  { Keyboard::KEY_RIGHTPAREN, love::window::ppapi::KEY_CODE_0 },
//  { Keyboard::KEY_ASTERISK, love::window::ppapi::KEY_CODE_8 },
  { Keyboard::KEY_PLUS, love::window::ppapi::KEY_CODE_OEM_PLUS },
  { Keyboard::KEY_COMMA, love::window::ppapi::KEY_CODE_OEM_COMMA },
  { Keyboard::KEY_MINUS, love::window::ppapi::KEY_CODE_OEM_MINUS },
  { Keyboard::KEY_PERIOD, love::window::ppapi::KEY_CODE_OEM_PERIOD },
  { Keyboard::KEY_SLASH, love::window::ppapi::KEY_CODE_OEM_2 },
  { Keyboard::KEY_0, love::window::ppapi::KEY_CODE_0 },
  { Keyboard::KEY_1, love::window::ppapi::KEY_CODE_1 },
  { Keyboard::KEY_2, love::window::ppapi::KEY_CODE_2 },
  { Keyboard::KEY_3, love::window::ppapi::KEY_CODE_3 },
  { Keyboard::KEY_4, love::window::ppapi::KEY_CODE_4 },
  { Keyboard::KEY_5, love::window::ppapi::KEY_CODE_5 },
  { Keyboard::KEY_6, love::window::ppapi::KEY_CODE_6 },
  { Keyboard::KEY_7, love::window::ppapi::KEY_CODE_7 },
  { Keyboard::KEY_8, love::window::ppapi::KEY_CODE_8 },
  { Keyboard::KEY_9, love::window::ppapi::KEY_CODE_9 },
//  { Keyboard::KEY_COLON, love::window::ppapi::KEY_CODE_OEM_1 },
  { Keyboard::KEY_SEMICOLON, love::window::ppapi::KEY_CODE_OEM_1 },
//  { Keyboard::KEY_LESS, love::window::ppapi::KEY_CODE_OEM_COMMA },
  { Keyboard::KEY_EQUALS, love::window::ppapi::KEY_CODE_OEM_PLUS },
//  { Keyboard::KEY_GREATER, love::window::ppapi::KEY_CODE_OEM_PERIOD },
//  { Keyboard::KEY_QUESTION, love::window::ppapi::KEY_CODE_OEM_2 },
//  { Keyboard::KEY_AT, love::window::ppapi::KEY_CODE_2 },

  { Keyboard::KEY_LEFTBRACKET, love::window::ppapi::KEY_CODE_OEM_4 },
  { Keyboard::KEY_BACKSLASH, love::window::ppapi::KEY_CODE_OEM_5 },
  { Keyboard::KEY_RIGHTBRACKET, love::window::ppapi::KEY_CODE_OEM_6 },
//  { Keyboard::KEY_CARET, love::window::ppapi::KEY_CODE_6 },
  { Keyboard::KEY_UNDERSCORE, love::window::ppapi::KEY_CODE_OEM_MINUS },
  { Keyboard::KEY_BACKQUOTE, love::window::ppapi::KEY_CODE_OEM_3 },
  { Keyboard::KEY_A, love::window::ppapi::KEY_CODE_A },
  { Keyboard::KEY_B, love::window::ppapi::KEY_CODE_B },
  { Keyboard::KEY_C, love::window::ppapi::KEY_CODE_C },
  { Keyboard::KEY_D, love::window::ppapi::KEY_CODE_D },
  { Keyboard::KEY_E, love::window::ppapi::KEY_CODE_E },
  { Keyboard::KEY_F, love::window::ppapi::KEY_CODE_F },
  { Keyboard::KEY_G, love::window::ppapi::KEY_CODE_G },
  { Keyboard::KEY_H, love::window::ppapi::KEY_CODE_H },
  { Keyboard::KEY_I, love::window::ppapi::KEY_CODE_I },
  { Keyboard::KEY_J, love::window::ppapi::KEY_CODE_J },
  { Keyboard::KEY_K, love::window::ppapi::KEY_CODE_K },
  { Keyboard::KEY_L, love::window::ppapi::KEY_CODE_L },
  { Keyboard::KEY_M, love::window::ppapi::KEY_CODE_M },
  { Keyboard::KEY_N, love::window::ppapi::KEY_CODE_N },
  { Keyboard::KEY_O, love::window::ppapi::KEY_CODE_O },
  { Keyboard::KEY_P, love::window::ppapi::KEY_CODE_P },
  { Keyboard::KEY_Q, love::window::ppapi::KEY_CODE_Q },
  { Keyboard::KEY_R, love::window::ppapi::KEY_CODE_R },
  { Keyboard::KEY_S, love::window::ppapi::KEY_CODE_S },
  { Keyboard::KEY_T, love::window::ppapi::KEY_CODE_T },
  { Keyboard::KEY_U, love::window::ppapi::KEY_CODE_U },
  { Keyboard::KEY_V, love::window::ppapi::KEY_CODE_V },
  { Keyboard::KEY_W, love::window::ppapi::KEY_CODE_W },
  { Keyboard::KEY_X, love::window::ppapi::KEY_CODE_X },
  { Keyboard::KEY_Y, love::window::ppapi::KEY_CODE_Y },
  { Keyboard::KEY_Z, love::window::ppapi::KEY_CODE_Z },
  { Keyboard::KEY_DELETE, love::window::ppapi::KEY_CODE_DELETE },

  { Keyboard::KEY_KP0, love::window::ppapi::KEY_CODE_NUMPAD0 },
  { Keyboard::KEY_KP1, love::window::ppapi::KEY_CODE_NUMPAD1 },
  { Keyboard::KEY_KP2, love::window::ppapi::KEY_CODE_NUMPAD2 },
  { Keyboard::KEY_KP3, love::window::ppapi::KEY_CODE_NUMPAD3 },
  { Keyboard::KEY_KP4, love::window::ppapi::KEY_CODE_NUMPAD4 },
  { Keyboard::KEY_KP5, love::window::ppapi::KEY_CODE_NUMPAD5 },
  { Keyboard::KEY_KP6, love::window::ppapi::KEY_CODE_NUMPAD6 },
  { Keyboard::KEY_KP7, love::window::ppapi::KEY_CODE_NUMPAD7 },
  { Keyboard::KEY_KP8, love::window::ppapi::KEY_CODE_NUMPAD8 },
  { Keyboard::KEY_KP9, love::window::ppapi::KEY_CODE_NUMPAD9 },
  { Keyboard::KEY_KP_PERIOD, love::window::ppapi::KEY_CODE_DECIMAL },
  { Keyboard::KEY_KP_DIVIDE, love::window::ppapi::KEY_CODE_DIVIDE },
  { Keyboard::KEY_KP_MULTIPLY, love::window::ppapi::KEY_CODE_MULTIPLY},
  { Keyboard::KEY_KP_MINUS, love::window::ppapi::KEY_CODE_SUBTRACT },
  { Keyboard::KEY_KP_PLUS, love::window::ppapi::KEY_CODE_ADD },
//  { Keyboard::KEY_KP_ENTER, love::window::ppapi::KEY_CODE_RETURN },
//  { Keyboard::KEY_KP_EQUALS, love::window::ppapi::KEY_CODE_OEM_PLUS },

  { Keyboard::KEY_UP, love::window::ppapi::KEY_CODE_UP },
  { Keyboard::KEY_DOWN, love::window::ppapi::KEY_CODE_DOWN },
  { Keyboard::KEY_RIGHT, love::window::ppapi::KEY_CODE_RIGHT },
  { Keyboard::KEY_LEFT, love::window::ppapi::KEY_CODE_LEFT },
  { Keyboard::KEY_INSERT, love::window::ppapi::KEY_CODE_INSERT },
  { Keyboard::KEY_HOME, love::window::ppapi::KEY_CODE_HOME },
  { Keyboard::KEY_END, love::window::ppapi::KEY_CODE_END },
  { Keyboard::KEY_PAGEUP, love::window::ppapi::KEY_CODE_PRIOR },
  { Keyboard::KEY_PAGEDOWN, love::window::ppapi::KEY_CODE_NEXT },

  { Keyboard::KEY_F1, love::window::ppapi::KEY_CODE_F1 },
  { Keyboard::KEY_F2, love::window::ppapi::KEY_CODE_F2 },
  { Keyboard::KEY_F3, love::window::ppapi::KEY_CODE_F3 },
  { Keyboard::KEY_F4, love::window::ppapi::KEY_CODE_F4 },
  { Keyboard::KEY_F5, love::window::ppapi::KEY_CODE_F5 },
  { Keyboard::KEY_F6, love::window::ppapi::KEY_CODE_F6 },
  { Keyboard::KEY_F7, love::window::ppapi::KEY_CODE_F7 },
  { Keyboard::KEY_F8, love::window::ppapi::KEY_CODE_F8 },
  { Keyboard::KEY_F9, love::window::ppapi::KEY_CODE_F9 },
  { Keyboard::KEY_F10, love::window::ppapi::KEY_CODE_F10 },
  { Keyboard::KEY_F11, love::window::ppapi::KEY_CODE_F11 },
  { Keyboard::KEY_F12, love::window::ppapi::KEY_CODE_F12 },
  { Keyboard::KEY_F13, love::window::ppapi::KEY_CODE_F13 },
  { Keyboard::KEY_F14, love::window::ppapi::KEY_CODE_F14 },
  { Keyboard::KEY_F15, love::window::ppapi::KEY_CODE_F15 },

  { Keyboard::KEY_NUMLOCK, love::window::ppapi::KEY_CODE_NUMLOCK },
  { Keyboard::KEY_CAPSLOCK, love::window::ppapi::KEY_CODE_CAPITAL },
  { Keyboard::KEY_SCROLLOCK, love::window::ppapi::KEY_CODE_SCROLL },
  { Keyboard::KEY_RSHIFT, love::window::ppapi::KEY_CODE_SHIFT },
  { Keyboard::KEY_LSHIFT, love::window::ppapi::KEY_CODE_SHIFT },
  { Keyboard::KEY_RCTRL, love::window::ppapi::KEY_CODE_CONTROL },
  { Keyboard::KEY_LCTRL, love::window::ppapi::KEY_CODE_CONTROL },
  { Keyboard::KEY_RALT, love::window::ppapi::KEY_CODE_MENU },
  { Keyboard::KEY_LALT, love::window::ppapi::KEY_CODE_MENU },
//  { Keyboard::KEY_RMETA, love::window::ppapi::KEY_CODE_RMETA },
//  { Keyboard::KEY_LMETA, love::window::ppapi::KEY_CODE_LMETA },
  { Keyboard::KEY_LSUPER, love::window::ppapi::KEY_CODE_LWIN },
  { Keyboard::KEY_RSUPER, love::window::ppapi::KEY_CODE_RWIN },
//  { Keyboard::KEY_MODE, love::window::ppapi::KEY_CODE_MODE },
//  { Keyboard::KEY_COMPOSE, love::window::ppapi::KEY_CODE_COMPOSE },

  { Keyboard::KEY_HELP, love::window::ppapi::KEY_CODE_HELP },
  { Keyboard::KEY_PRINT, love::window::ppapi::KEY_CODE_SNAPSHOT },
//  { Keyboard::KEY_SYSREQ, love::window::ppapi::KEY_CODE_SYSREQ },
//  { Keyboard::KEY_BREAK, love::window::ppapi::KEY_CODE_BREAK },
//  { Keyboard::KEY_MENU, love::window::ppapi::KEY_CODE_MENU },
//  { Keyboard::KEY_POWER, love::window::ppapi::KEY_CODE_POWER },
//  { Keyboard::KEY_EURO, love::window::ppapi::KEY_CODE_EURO },
//  { Keyboard::KEY_UNDO, love::window::ppapi::KEY_CODE_UNDO },
};

Keyboard::KeyEnumMap Keyboard::keys(Keyboard::keyEntries,
                                    sizeof(Keyboard::keyEntries));

} // ppapi
} // keyboard
} // love
