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
    msg = convert(*iter);
    if (msg) {
      push(convert(*iter));
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

    case INPUT_KEY:
      switch (event.key.type) {
        case KEY_DOWN:
        case KEY_UP: {
          love::keyboard::Keyboard::Key key;
          if (love::keyboard::ppapi::Keyboard::Convert((KeyCode) event.key.code, &key) &&
              love::event::Event::keys.find(key, txt)) {
            arg1 = new Variant(txt, strlen(txt));
            arg2 = new Variant((double) 0);  // TODO(binji): fix
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

