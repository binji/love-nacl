#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include <string>
#include <sys/mount.h>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>
#include "GLES2/gl2.h"
#include <nacl_io/nacl_io.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/var.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>
#include <ppapi/utility/completion_callback_factory.h>
#include <ppapi/utility/threading/simple_thread.h>

#include <window/Window.h>
#include <window/ppapi/Window.h>
#include "FilesystemHack.h"
#include "Input.h"

#define READ_BUFFER_SIZE (128 * 1024)


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
  virtual void DidChangeFocus(bool has_focus);
  virtual bool HandleDocumentLoad(const pp::URLLoader& url_loader);
  virtual bool HandleInputEvent(const pp::InputEvent& event);
  virtual void HandleMessage(const pp::Var& message);

  void PostMessagef(const char* format, ...);

 private:
  void MainLoop_Initialize(int32_t);
  void MainLoop_Download();
  void MainLoop_Run(int32_t);
  void Filesystem_AllowAccess(int32_t, bool allowed);
  void Filesystem_CopyFile(int32_t, const std::string& path);
  void Filesystem_MakeDir(int32_t, const std::string& path);

  std::string url_;

  pp::URLRequestInfo url_request_;
  pp::URLLoader url_loader_;
  char* buffer_;

  pp::CompletionCallbackFactory<Instance> callback_factory_;
  pp::SimpleThread main_loop_thread_;
  pp::SimpleThread filesystem_thread_;
};

Instance::Instance(PP_Instance instance)
    : pp::Instance(instance),
      buffer_(new char[READ_BUFFER_SIZE]),
      callback_factory_(this),
      main_loop_thread_(this),
      filesystem_thread_(this) {
  PPB_GetInterface get_browser_interface =
      pp::Module::Get()->get_browser_interface();
  glInitializePPAPI(get_browser_interface);
  nacl_io_init_ppapi(instance, get_browser_interface);
  alSetPpapiInfo(instance, get_browser_interface);

  g_Instance = this;
}

Instance::~Instance() {
  delete [] buffer_;
}

bool Instance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_WHEEL);
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);

  InitializeEventQueue();
  InitializeFilesystemHack();
  main_loop_thread_.Start();
  filesystem_thread_.Start();

  std::string src;
  std::string love_src;
  for (uint32_t i = 0; i < argc; ++i) {
    if (!strcmp(argn[i], "love_src")) {
      love_src = argv[i];
      printf("Found love_src: %s\n", love_src.c_str());
    } else if (!strcmp(argn[i], "src")) {
      src = argv[i];
      printf("Found src: %s\n", src.c_str());
    }
  }

  if (!love_src.empty()) {
    // Loading via dropped file, etc. Start MainLoop immediately.
    src = love_src;
    main_loop_thread_.message_loop().PostWork(
        callback_factory_.NewCallback(&Instance::MainLoop_Initialize));
  } else {
    // Loading using HandleDocumentLoad, wait for call before starting MainLoop.
  }

  url_ = src;

  mount("", "/temporary", "memfs", 0, "");
  mount("", "/persistent", "memfs", 0, "");
  // HACK(binji): copy from persistent to html5fs to backup... once html5fs can
  // work with physfs, get rid of this nastiness.
  mount("", "/html5fs", "html5fs", 0, "type=PERSISTENT");
  return true;
}

void Instance::DidChangeView(const pp::View& view) {
  pp::Rect rect = view.GetRect();
  EnqueueViewChangeEvent(rect.width(), rect.height());
}

void Instance::DidChangeFocus(bool has_focus) {
  EnqueueFocusEvent(has_focus);
}

bool Instance::HandleDocumentLoad(const pp::URLLoader& url_loader) {
  url_loader_ = url_loader;
  main_loop_thread_.message_loop().PostWork(
      callback_factory_.NewCallback(&Instance::MainLoop_Initialize));
  return true;
}

bool Instance::HandleInputEvent(const pp::InputEvent& event) {
  EnqueueEvent(event);
  return true;
}

void Instance::HandleMessage(const pp::Var& var) {
  if (!var.is_string()) {
    return;
  }

  std::string message = var.AsString();
  std::string command;
  std::string params;
  size_t colon = message.find(':');
  if (colon != std::string::npos) {
    command = message.substr(0, colon);
    params = message.substr(colon + 1);
  } else {
    command = message;
  }

  printf("Got command: %s params: %s\n", command.c_str(), params.c_str());
  if (command == "fileSystemAccess") {
    bool allowed = params == "yes";
    filesystem_thread_.message_loop().PostWork(
        callback_factory_.NewCallback(
            &Instance::Filesystem_AllowAccess, allowed));
  } else if (command == "copyFile") {
    filesystem_thread_.message_loop().PostWork(
        callback_factory_.NewCallback(&Instance::Filesystem_CopyFile, params));
  } else if (command == "makeDir") {
    filesystem_thread_.message_loop().PostWork(
        callback_factory_.NewCallback(&Instance::Filesystem_MakeDir, params));
  } else if (command == "run") {
    main_loop_thread_.message_loop().PostWork(
        callback_factory_.NewCallback(&Instance::MainLoop_Run));
  } else {
    fprintf(stderr, "Unknown message: %s\n", message.c_str());
  }
}

void Instance::PostMessagef(const char* format, ...) {
  const size_t kBufferSize = 1024;
  char buffer[kBufferSize];
  va_list args;
  va_start(args, format);
  vsnprintf(&buffer[0], kBufferSize, format, args);

  PostMessage(buffer);
}

void Instance::MainLoop_Initialize(int32_t) {
  // redirect stdout/stderr to console.
  int fd1 = open("/dev/console0", O_WRONLY);
  int fd2 = open("/dev/console3", O_WRONLY);
  dup2(fd1, 1);
  dup2(fd2, 2);
  setvbuf(stdout, NULL, _IOLBF, 0);
  setvbuf(stderr, NULL, _IOLBF, 0);

  MainLoop_Download();

  // Notify the JavaScript that we're OK!
  PostMessage("OK");
}

void Instance::MainLoop_Download() {
  int32_t result;
  if (url_loader_.is_null()) {
    printf("Download: url_loader_ is null?\n");
    url_request_ = pp::URLRequestInfo(this);
    url_request_.SetURL(url_.c_str());
    url_request_.SetMethod("GET");
    url_request_.SetRecordDownloadProgress(true);
    url_loader_ = pp::URLLoader(this);
    result = url_loader_.Open(url_request_, pp::CompletionCallback());
    if (result != PP_OK) {
      fprintf(stderr, "Cannot read from URL %s. Error %d\n", url_.c_str(),
              result);
      return;
    }
  }

  int64_t total_received = 0;
  int64_t total_bytes = 0;
  int64_t total_written = 0;

  FILE* outf = fopen("/temporary/game.love", "w+");
  if (!outf) {
    fprintf(stderr, "Cannot open output file\n");
    return;
  }

  while (1) {
    if (url_loader_.GetDownloadProgress(&total_received, &total_bytes)) {
      PostMessagef("download:%lld,%lld", total_received, total_bytes);
    } else {
      PostMessagef("download:%lld,0", total_written);
    }

    result = url_loader_.ReadResponseBody(&buffer_[0], READ_BUFFER_SIZE,
                                          pp::CompletionCallback());
    if (result < 0) {
      fprintf(stderr, "Error reading from URL %s. Error %d\n", url_.c_str(),
              result);
      goto done;
    } else if (result == 0) {
      break;
    }

    size_t bytes_written = fwrite(&buffer_[0], 1, result, outf);
    if (bytes_written != static_cast<size_t>(result)) {
      fprintf(stderr, "Error writing to output file\n");
      goto done;
    }

    total_written += bytes_written;
  }

  printf("Done.\n");

done:
  fclose(outf);
  url_loader_ = pp::URLLoader();
  return;
}

void Instance::MainLoop_Run(int32_t) {
  std::vector<const char*> args;
  args.push_back("/");
  args.push_back("/temporary/game.love");
  love_main(args.size(), const_cast<char**>(args.data()));
  PostMessage("bye");
}

void Instance::Filesystem_AllowAccess(int32_t, bool allowed) {
  SetFilesystemAccessAllowed(allowed);
}

void Instance::Filesystem_CopyFile(int32_t, const std::string& path) {
  CopyFileForRead(path.c_str());
}

void Instance::Filesystem_MakeDir(int32_t, const std::string& path) {
  MakeDirectoryForRead(path.c_str());
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
