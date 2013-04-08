#include <AL/al.h>
#include <AL/alc.h>
#include <nacl_io/nacl_io.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/module.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

#include <window/Window.h>
#include "Input.h"


int love_main(int argc, char** argv);

namespace love {
namespace window {
namespace ppapi {

pp::Instance* g_Instance = NULL;

class Instance : public pp::Instance {
 public:
  explicit Instance(PP_Instance instance);
  virtual ~Instance();

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view);
  virtual bool HandleInputEvent(const pp::InputEvent& event);

 private:
  static void* MainLoop(void*);

  pthread_t main_loop_thread_;
};

Instance::Instance(PP_Instance instance)
    : pp::Instance(instance) {
  PPB_GetInterface get_browser_interface =
      pp::Module::Get()->get_browser_interface();
  glInitializePPAPI(get_browser_interface);
  nacl_io_init_ppapi(instance, get_browser_interface);
  alSetPpapiInfo(instance, get_browser_interface);

  g_Instance = this;
}

Instance::~Instance() {
}

bool Instance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);

  InitializeEventQueue();

  // Mount HTTPFS at /http. All file requests are relative to the .nmf
  mount("", "/http", "httpfs", 0, "");
  pthread_create(&main_loop_thread_, NULL, MainLoop, this);
}

void Instance::DidChangeView(const pp::View& view) {
  // TODO(binji): what to do here?
}

bool Instance::HandleInputEvent(const pp::InputEvent& event) {
  EnqueueEvent(event);
  return true;
}

void* Instance::MainLoop(void* param) {
  static const char* args[] = { "/", "--game", "/http/exo-slime.love" };
  love_main(sizeof(args)/sizeof(args[0]), const_cast<char**>(&args[0]));
}


class Module : public pp::Module {
 public:
  Module() : pp::Module() {}
  virtual ~Module() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new Instance(instance);
  }
};

}  // namespace ppapi
}  // namespace window
}  // namespace love

namespace pp {
Module* CreateModule() {
  return new love::window::ppapi::Module();
}
}  // namespace pp
