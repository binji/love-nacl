#include "Input.h"

#include <deque>
#include <string.h>
#include <ppapi/cpp/var.h>
#include <pthread.h>
#include "Window.h"

namespace love {
namespace window {
namespace ppapi {


int g_mouse_x;
int g_mouse_y;
bool g_mouse_button[MOUSE_BUTTON_MAX];
bool g_keys[KEY_CODE_MAX];
bool g_key_repeat = false;

typedef std::deque<InputEvent> InputEventQueue;
InputEventQueue g_input_event_queue;
pthread_mutex_t g_event_queue_mutex;
pthread_cond_t g_queue_non_empty;

void UpdateInputState(const InputEvent& event);
void UpdateInputState(const InputEvents& events);


bool ConvertEvent(const pp::InputEvent& in_event, InputEvent* out_event) {
  InputType type;
  switch (in_event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
      type = INPUT_MOUSE;
      out_event->mouse.type = MOUSE_DOWN;
      break;
    case PP_INPUTEVENT_TYPE_MOUSEUP:
      type = INPUT_MOUSE;
      out_event->mouse.type = MOUSE_UP;
      break;
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
      type = INPUT_MOUSE;
      out_event->mouse.type = MOUSE_MOVE;
      break;
    case PP_INPUTEVENT_TYPE_MOUSEENTER:
      type = INPUT_MOUSE;
      out_event->mouse.type = MOUSE_ENTER;
      break;
    case PP_INPUTEVENT_TYPE_MOUSELEAVE:
      type = INPUT_MOUSE;
      out_event->mouse.type = MOUSE_LEAVE;
      break;
    case PP_INPUTEVENT_TYPE_WHEEL:
      type = INPUT_WHEEL;
      break;
    case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
      type = INPUT_KEY;
      out_event->key.type = RAW_KEY_DOWN;
      break;
    case PP_INPUTEVENT_TYPE_KEYDOWN:
      type = INPUT_KEY;
      out_event->key.type = KEY_DOWN;
      break;
    case PP_INPUTEVENT_TYPE_KEYUP:
      type = INPUT_KEY;
      out_event->key.type = KEY_UP;
      break;
    case PP_INPUTEVENT_TYPE_CHAR:
      type = INPUT_CHARACTER;
      break;
    default:
      return false;
  }

  out_event->type = type;
  out_event->modifiers = in_event.GetModifiers();
  switch (type) {
    case INPUT_MOUSE: {
      pp::MouseInputEvent mouse_event(in_event);
      out_event->mouse.x = mouse_event.GetPosition().x();
      out_event->mouse.y = mouse_event.GetPosition().y();
      out_event->mouse.movement_x = mouse_event.GetMovement().x();
      out_event->mouse.movement_y = mouse_event.GetMovement().y();
      switch (mouse_event.GetButton()) {
        case PP_INPUTEVENT_MOUSEBUTTON_NONE:
          out_event->mouse.button = MOUSE_NONE;
          break;
        case PP_INPUTEVENT_MOUSEBUTTON_LEFT:
          out_event->mouse.button = MOUSE_LEFT;
          break;
        case PP_INPUTEVENT_MOUSEBUTTON_MIDDLE:
          out_event->mouse.button = MOUSE_MIDDLE;
          break;
        case PP_INPUTEVENT_MOUSEBUTTON_RIGHT:
          out_event->mouse.button = MOUSE_RIGHT;
          break;
      }
      break;
    }
    case INPUT_WHEEL: {
      pp::WheelInputEvent wheel_event(in_event);
      out_event->wheel.delta_x = wheel_event.GetDelta().x();
      out_event->wheel.delta_y = wheel_event.GetDelta().y();
      out_event->wheel.ticks_x = wheel_event.GetTicks().x();
      out_event->wheel.ticks_y = wheel_event.GetTicks().y();
      out_event->wheel.scroll_by_page = wheel_event.GetScrollByPage();
      break;
    }
    case INPUT_KEY: {
      pp::KeyboardInputEvent key_event(in_event);
      out_event->key.code = key_event.GetKeyCode();
      memset(out_event->key.text, 0, sizeof(out_event->key.text));

      // Kill repeated keys if it is turned off.
      if (!g_key_repeat) {
        if (out_event->key.type == KEY_DOWN &&
            IsKeyPressed(out_event->key.code))
            return false;
      }
      break;
    }
    case INPUT_CHARACTER: {
      pp::KeyboardInputEvent key_event(in_event);
      strncpy(out_event->character.text,
              key_event.GetCharacterText().AsString().c_str(),
              sizeof(out_event->key.text));
      break;
    }
  }

  return true;
}

void InitializeEventQueue() {
  pthread_mutex_init(&g_event_queue_mutex, NULL);
  pthread_cond_init(&g_queue_non_empty, NULL);
}

void EnqueueFocusEvent(bool has_focus) {
  InputEvent event;
  event.type = INPUT_FOCUS;
  event.focus.has_focus = has_focus;
  EnqueueEvent(event);
}

void EnqueueEvent(const pp::InputEvent& event) {
  InputEvent converted_event;
  if (ConvertEvent(event, &converted_event))
    EnqueueEvent(converted_event);
}

void EnqueueEvent(const InputEvent& event) {
  pthread_mutex_lock(&g_event_queue_mutex);
  // Try to combine this character event with a previous key event.
  // It's possible that we've already sent off the INPUT_KEY event without
  // attaching its accompanying INPUT_CHARACTER event, but it's still better
  // than waiting for an INPUT_CHARACTER event when we receive an INPUT_KEY
  // event.
  if (event.type == INPUT_CHARACTER) {
    if (!g_input_event_queue.empty()) {
      InputEvent& prev_event = g_input_event_queue.back();
      if (prev_event.type == INPUT_KEY && prev_event.key.type == KEY_DOWN) {
        strncpy(prev_event.key.text, event.character.text,
                sizeof(prev_event.key.text));
        pthread_mutex_unlock(&g_event_queue_mutex);
        return;
      }
    }
  }

  g_input_event_queue.push_back(event);
  pthread_cond_signal(&g_queue_non_empty);
  pthread_mutex_unlock(&g_event_queue_mutex);
}

bool DequeueEvent(InputEvent* out_event) {
  bool has_event = false;
  pthread_mutex_lock(&g_event_queue_mutex);
  if (!g_input_event_queue.empty()) {
    *out_event = g_input_event_queue.front();
    g_input_event_queue.pop_front();
    has_event = true;
  }
  pthread_mutex_unlock(&g_event_queue_mutex);
  UpdateInputState(*out_event);
  return has_event;
}

void DequeueAllEvents(InputEvents* out_events) {
  pthread_mutex_lock(&g_event_queue_mutex);
  out_events->resize(g_input_event_queue.size());
  std::copy(g_input_event_queue.begin(), g_input_event_queue.end(),
            out_events->begin());
  g_input_event_queue.clear();
  pthread_mutex_unlock(&g_event_queue_mutex);
  UpdateInputState(*out_events);
}

void WaitForEvent() {
  pthread_mutex_lock(&g_event_queue_mutex);
  while (g_input_event_queue.empty()) {
    pthread_cond_wait(&g_queue_non_empty, &g_event_queue_mutex);
  }
  pthread_mutex_unlock(&g_event_queue_mutex);
}

int GetMouseX() {
  return g_mouse_x;
}

int GetMouseY() {
  return g_mouse_y;
}

bool IsMouseButtonPressed(MouseButton button) {
  if (button <= MOUSE_NONE || button >= MOUSE_BUTTON_MAX)
    return false;
  return g_mouse_button[button];
}

bool IsKeyPressed(uint32_t code) {
  if (code >= KEY_CODE_MAX)
    return false;
  return g_keys[code];
}

void SetKeyRepeat(bool repeat) {
  g_key_repeat = repeat;
}

void UpdateInputState(const InputEvents& events) {
  for (InputEvents::const_iterator iter = events.begin();
       iter != events.end();
       ++iter) {
    UpdateInputState(*iter);
  }
}

void UpdateInputState(const InputEvent& event) {
  Window* window = static_cast<Window*>(Window::getSingleton());
  switch (event.type) {
    case INPUT_MOUSE:
      g_mouse_x = event.mouse.x;
      g_mouse_y = event.mouse.y;

      switch (event.mouse.type) {
        case MOUSE_DOWN:
          g_mouse_button[event.mouse.button] = true;
          break;
        case MOUSE_UP:
          g_mouse_button[event.mouse.button] = false;
          break;
      }
      break;

    case INPUT_KEY:
      switch (event.key.type) {
        case KEY_DOWN:
          if (event.key.code < KEY_CODE_MAX)
            g_keys[event.key.code] = true;
          break;
        case KEY_UP:
          if (event.key.code < KEY_CODE_MAX)
            g_keys[event.key.code] = false;
          break;
      }
      break;

    case INPUT_FOCUS:
      window->onFocusChanged(event.focus.has_focus);
      break;
  }
}

}  // ppapi
}  // window
}  // love
