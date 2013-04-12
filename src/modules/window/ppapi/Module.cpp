#include <string>
#include <vector>
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
  std::string game_;
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

  std::string src;
  std::string love_src;
  for (uint32_t i = 0; i < argc; ++i) {
    if (!strcmp(argn[i], "love_src")) {
      src = argv[i];
      printf("Found love_src: %s\n", src.c_str());
    } else if (!strcmp(argn[i], "src")) {
      src = argv[i];
      printf("Found src: %s\n", src.c_str());
    }
  }

  if (!love_src.empty())
    src = love_src;

  std::string base = "/";
  if (!src.empty()) {
    size_t last_slash = src.find_last_of('/');
    if (last_slash != std::string::npos) {
      base = src.substr(0, last_slash);
      game_ = "/http/" + src.substr(last_slash + 1);
    } else {
      base = "";
      game_ = "/http/" + src;
    }
  }

  printf("Game = %s\n", game_.c_str());

  mount(base.c_str(), "/http", "httpfs", 0, "");
  pthread_create(&main_loop_thread_, NULL, MainLoop, this);
}

void Instance::DidChangeView(const pp::View& view) {
  pp::Size size = view.GetRect().size();
  InputEvent event;
  event.type = INPUT_SCREEN_CHANGED;
  event.screen_changed.width = size.width();
  event.screen_changed.height = size.height();
  EnqueueEvent(event);
}

bool Instance::HandleInputEvent(const pp::InputEvent& event) {
  EnqueueEvent(event);
  return true;
}

void* Instance::MainLoop(void* param) {
  Instance* instance = static_cast<Instance*>(param);
  std::vector<const char*> args;
  args.push_back("/");
  if (!instance->game_.empty())
    args.push_back(instance->game_.c_str());
  love_main(args.size(), const_cast<char**>(args.data()));
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
