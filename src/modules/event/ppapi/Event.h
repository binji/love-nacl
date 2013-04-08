/**
 **/

#ifndef LOVE_EVENT_PPAPI_EVENT_H
#define LOVE_EVENT_PPAPI_EVENT_H

// LOVE
#include <event/Event.h>
#include <common/runtime.h>
#include <common/EnumMap.h>
#include <window/ppapi/Input.h>

namespace love {
namespace event {
namespace ppapi {

class Event : public love::event::Event {
  public:
    // Implements Module.
    const char * getName() const;

    Event();

    void pump();
    void clear();
    Message* wait();

  private:
    Message* convert(const love::window::ppapi::InputEvent& event);

    /*
     static EnumMap<love::keyboard::Keyboard::Key, SDLKey, love::keyboard::Keyboard::KEY_MAX_ENUM>::Entry keyEntries[];
     static EnumMap<love::keyboard::Keyboard::Key, SDLKey, love::keyboard::Keyboard::KEY_MAX_ENUM> keys;
       */
    typedef love::mouse::Mouse::Button LoveMouseButton;
    typedef love::window::ppapi::MouseButton PPAPIMouseButton;
    typedef EnumMap<LoveMouseButton, PPAPIMouseButton, love::window::ppapi::MOUSE_BUTTON_MAX> MouseEnumMap;
    static MouseEnumMap::Entry buttonEntries[];
    static MouseEnumMap buttons;
}; // System

} // ppapi
} // event
} // love

#endif // LOVE_EVENT_PPAPI_EVENT_H

