/**
**/

#include "Mouse.h"
#include <window/ppapi/Input.h>

namespace love {
namespace mouse {
namespace ppapi {

const char * Mouse::getName() const {
  return "love.mouse.ppapi";
}

int Mouse::getX() const {
  return love::window::ppapi::GetMouseX();
}

int Mouse::getY() const {
  return love::window::ppapi::GetMouseY();
}

void Mouse::getPosition(int & x, int & y) const {
  x = love::window::ppapi::GetMouseX();
  y = love::window::ppapi::GetMouseY();
}

void Mouse::setPosition(int x, int y) {
}

void Mouse::setVisible(bool visible) {
}

bool Mouse::isDown(Button * buttonlist) const {
  using namespace love::window::ppapi;
  for (Button button = *buttonlist;
       button != BUTTON_MAX_ENUM;
       button = *(++buttonlist)) {
    MouseButton ppapi_button = MOUSE_NONE;
    switch (button) {
      case BUTTON_LEFT: ppapi_button = MOUSE_LEFT; break;
      case BUTTON_RIGHT: ppapi_button = MOUSE_RIGHT; break;
      case BUTTON_MIDDLE: ppapi_button = MOUSE_MIDDLE; break;
    }

    if (IsMouseButtonPressed(ppapi_button))
      return true;
  }

  return false;
}

bool Mouse::isVisible() const {
  return true;
}

void Mouse::setGrab(bool grab) {
}

bool Mouse::isGrabbed() const {
  return false;
}

} // ppapi
} // mouse
} // love
