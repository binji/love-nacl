#ifndef _SDL_nacl_h
#define _SDL_nacl_h

#include "begin_code.h"
#include <ppapi/cpp/input_event.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include <ppapi/c/ppp_instance.h>
void SDL_NACL_SetInstance(PP_Instance instance, int width, int height);
void SDL_NACL_PushEvent(const pp::InputEvent& ppevent);
void SDL_NACL_SetHasFocus(bool has_focus);
void SDL_NACL_SetPageVisible(bool is_visible);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_nacl_h */
