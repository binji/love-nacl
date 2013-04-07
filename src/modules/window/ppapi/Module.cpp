#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

#include <window/Window.h>

int love_main(int argc, char** argv);

namespace love {
namespace window {
namespace ppapi {

pp::Instance* g_Instance = NULL;

class Instance : public pp::Instance {
 public:
  explicit Instance(PP_Instance instance);
  virtual ~Instance();

  virtual void DidChangeView(const pp::View& view);

 private:
  static void* MainLoop(void*);

  pthread_t main_loop_thread_;
};

Instance::Instance(PP_Instance instance)
    : pp::Instance(instance) {
  glInitializePPAPI(pp::Module::Get()->get_browser_interface());
  g_Instance = this;
  pthread_create(&main_loop_thread_, NULL, MainLoop, this);
}

Instance::~Instance() {
}

void Instance::DidChangeView(const pp::View& view) {
}

void* Instance::MainLoop(void* param) {
  static const char* args[] = { "/" };
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
