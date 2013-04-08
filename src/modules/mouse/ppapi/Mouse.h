/**
**/

#ifndef LOVE_MOUSE_PPAPI_MOUSE_H
#define LOVE_MOUSE_PPAPI_MOUSE_H

// LOVE
#include <mouse/Mouse.h>

namespace love {
namespace mouse {
namespace ppapi {

class Mouse : public love::mouse::Mouse
{
 public:
  // Implements Module.
  const char * getName() const;

  int getX() const;
  int getY() const;
  void getPosition(int & x, int & y) const;
  void setPosition(int x, int y);
  void setVisible(bool visible);
  bool isDown(Button * buttonlist) const;
  bool isVisible() const;
  void setGrab(bool grab);
  bool isGrabbed() const;
}; // Mouse

} // ppapi
} // mouse
} // love

#endif // LOVE_MOUSE_PPAPI_MOUSE_H
