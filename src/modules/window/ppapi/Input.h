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

struct KeyEvent {
  KeyType type;
  uint32_t code;
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

struct InputEvent {
  InputType type;
  uint32_t modifiers;
  union {
    KeyEvent key;
    CharacterEvent character;
    MouseEvent mouse;
    WheelEvent wheel;
  };
};

typedef std::vector<InputEvent> InputEvents;

void InitializeEventQueue();
void EnqueueEvent(const pp::InputEvent& event);
bool DequeueEvent(InputEvent* out_event);
void DequeueAllEvents(InputEvents* out_events);
void WaitForEvent();

// Mouse state
int GetMouseX();
int GetMouseY();
bool IsMouseButtonPressed(MouseButton);

}  // ppapi
}  // window
}  // love

#endif  // LOVE_WINDOW_PPAPI_INPUT_H
