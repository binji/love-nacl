/**
 **/

#include "Event.h"

#include <keyboard/Keyboard.h>
#include <keyboard/ppapi/Keyboard.h>
#include <mouse/Mouse.h>
#include <window/ppapi/Input.h>

namespace love {
namespace event {
namespace ppapi {

uint32_t DecodeUtf8(const char (&text)[5]) {
  // Bit pattern:
  // 1-byte 0xxxxxxx
  // 2-byte 110xxxxx 10xxxxxx
  // 3-byte 1110xxxx 10xxxxxx 10xxxxxx
  // 4-byte 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  // 5-byte 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
  if ((text[0] & 0x80) == 0) {
    // ASCII
    return text[0];
  } else if ((text[0] & 0xE0) == 0xC0) {
    // 2-bytes.
    return (text[0] & 0x1f) << 6 |
           (text[1] & 0x3f);
  } else if ((text[0] & 0xF0) == 0xE0) {
    // 3-bytes.
    return (text[0] & 0x0f) << 12 |
           (text[1] & 0x3f) << 6 |
           (text[2] & 0x3f);
  } else if ((text[0] & 0xF8) == 0xF0) {
    // 4-bytes.
    return (text[0] & 0x07) << 18 |
           (text[1] & 0x3f) << 12 |
           (text[2] & 0x3f) << 6 |
           (text[3] & 0x3f);
  } else if ((text[0] & 0xFC) == 0xF8) {
    // 5-bytes.
    return (text[0] & 0x03) << 24 |
           (text[1] & 0x3f) << 18 |
           (text[2] & 0x3f) << 12 |
           (text[3] & 0x3f) << 6;
           (text[4] & 0x3f);
  }
  return 0;
}

const char * Event::getName() const {
  return "love.event.ppapi";
}

Event::Event() {
}

void Event::pump() {
  using namespace love::window::ppapi;
  InputEvents events;
  DequeueAllEvents(&events);

  Message *msg;
  for (InputEvents::iterator iter = events.begin();
      iter != events.end();
      ++iter) {
    const InputEvent& event = *iter;
    msg = convert(event);
    if (msg) {
      push(msg);
      msg->release();
    }
  }
}

void Event::clear() {
  using namespace love::window::ppapi;
  InputEvents events;
  DequeueAllEvents(&events);
  love::event::Event::clear();
}

Message* Event::wait() {
  using namespace love::window::ppapi;
  WaitForEvent();
  InputEvent event;
  if (DequeueEvent(&event))
    return convert(event);
  return NULL;
}

Message* Event::convert(const love::window::ppapi::InputEvent& event) {
  using namespace love::window::ppapi;

  Message* msg = NULL;
  Variant *arg1 = NULL;
  Variant *arg2 = NULL;
  Variant *arg3 = NULL;
  const char* txt = NULL;

  switch (event.type) {
    case INPUT_MOUSE:
      switch (event.mouse.type) {
        case MOUSE_DOWN:
        case MOUSE_UP: {
          love::mouse::Mouse::Button button;
          if (buttons.find(event.mouse.button, button) &&
              love::event::Event::buttons.find(button, txt)) {
            arg1 = new Variant((double) event.mouse.x);
            arg2 = new Variant((double) event.mouse.y);
            arg3 = new Variant(txt, strlen(txt));
            const char* type = event.mouse.type == MOUSE_DOWN ?
                "mousepressed" : "mousereleased";
            msg = new Message(type, arg1, arg2, arg3);
          }
          break;
        }
      }
      break;

    case INPUT_WHEEL: {
        arg1 = new Variant((double) GetMouseX());
        arg2 = new Variant((double) GetMouseY());
        const char* type = event.wheel.delta_y > 0 ? "wu" : "wd";
        arg3 = new Variant(type, strlen(type));
        msg = new Message("mousepressed", arg1, arg2, arg3);
      }
      break;

    case INPUT_KEY:
      switch (event.key.type) {
        case KEY_DOWN:
        case KEY_UP: {
          love::keyboard::Keyboard::Key key;
          if (love::keyboard::ppapi::Keyboard::Convert((KeyCode) event.key.code, &key) &&
              love::event::Event::keys.find(key, txt)) {
            arg1 = new Variant(txt, strlen(txt));
            arg2 = new Variant((double) DecodeUtf8(event.key.text));
            const char* type = event.key.type == KEY_DOWN ?
                "keypressed" : "keyreleased";
            msg = new Message(type, arg1, arg2);
          }
          break;
        }
      }
      break;
  }

  if (arg1) arg1->release();
  if (arg2) arg2->release();
  if (arg3) arg3->release();

  return msg;
}

/*
   Message *Event::convert(SDL_Event & e)
   {
   Message *msg = NULL;
   love::keyboard::Keyboard::Key key;
   love::mouse::Mouse::Button button;
   Variant *arg1, *arg2, *arg3;
   const char *txt;
   switch(e.type)
   {
   case SDL_KEYDOWN:
   if (keys.find(e.key.keysym.sym, key) && love::event::Event::keys.find(key, txt))
   {
   arg1 = new Variant(txt, strlen(txt));
   arg2 = new Variant((double) e.key.keysym.unicode);
   msg = new Message("keypressed", arg1, arg2);
   arg1->release();
   arg2->release();
   }
   break;
   case SDL_KEYUP:
   if (keys.find(e.key.keysym.sym, key) && love::event::Event::keys.find(key, txt))
   {
   arg1 = new Variant(txt, strlen(txt));
   msg = new Message("keyreleased", arg1);
   arg1->release();
   }
   break;
   case SDL_MOUSEBUTTONDOWN:
   case SDL_MOUSEBUTTONUP:
   if (buttons.find(e.button.button, button) && love::event::Event::buttons.find(button, txt))
   {
   arg1 = new Variant((double) e.button.x);
   arg2 = new Variant((double) e.button.y);
   arg3 = new Variant(txt, strlen(txt));
   msg = new Message((e.type == SDL_MOUSEBUTTONDOWN) ?
   "mousepressed" : "mousereleased",
   arg1, arg2, arg3);
   arg1->release();
   arg2->release();
   arg3->release();
   }
   break;
   case SDL_JOYBUTTONDOWN:
   case SDL_JOYBUTTONUP:
   arg1 = new Variant((double) (e.jbutton.which+1));
   arg2 = new Variant((double) (e.jbutton.button+1));
   msg = new Message((e.type == SDL_JOYBUTTONDOWN) ?
   "joystickpressed" : "joystickreleased",
   arg1, arg2);
   arg1->release();
   arg2->release();
   break;
   case SDL_ACTIVEEVENT:
   arg1 = new Variant(e.active.gain != 0);
   if (e.active.state & SDL_APPINPUTFOCUS)
   msg = new Message("focus", arg1);
   arg1->release();
   break;
   case SDL_QUIT:
   msg = new Message("quit");
   break;
   }

   return msg;
   }
   */

  Event::MouseEnumMap::Entry Event::buttonEntries[] = {
    { love::mouse::Mouse::BUTTON_LEFT, love::window::ppapi::MOUSE_LEFT },
    { love::mouse::Mouse::BUTTON_MIDDLE, love::window::ppapi::MOUSE_MIDDLE },
    { love::mouse::Mouse::BUTTON_RIGHT, love::window::ppapi::MOUSE_RIGHT },
  };

  Event::MouseEnumMap Event::buttons(Event::buttonEntries, sizeof(Event::buttonEntries));

} // ppapi
} // event
} // love

