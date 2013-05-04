#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>

class Instance : public pp::Instance {
 public:
  explicit Instance(PP_Instance instance);
  virtual ~Instance();

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void HandleMessage(const pp::Var& message);
};

Instance::Instance(PP_Instance instance)
    : pp::Instance(instance) {
}

Instance::~Instance() {
}

bool Instance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  return true;
}

void Instance::HandleMessage(const pp::Var& message) {
  PostMessage("OK");
}

class Module : public pp::Module {
 public:
  Module() : pp::Module() {}
  virtual ~Module() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new ::Instance(instance);
  }
};

namespace pp {
Module* CreateModule() {
  return new ::Module();
}
}  // namespace pp
