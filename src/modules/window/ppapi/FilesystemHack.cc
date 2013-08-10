#include "FilesystemHack.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <deque>
#include <string>

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/var.h"

#include "sdk_util/auto_lock.h"

namespace love {
namespace window {
namespace ppapi {

extern pp::Instance* g_Instance;

namespace {

const char kFakePersistentDir[] = "/persistent";
const char kRealPersistentDir[] = "/html5fs";
const char kRequestFilesystemMessage[] = "requestFileSystem";

enum FilesystemAccess {
  FILESYSTEM_ACCESS_UNKNOWN,
  FILESYSTEM_ACCESS_ALLOWED,
  FILESYSTEM_ACCESS_DISALLOWED,
};

static bool s_requested_filesystem_access = false;
static FilesystemAccess s_filesystem_access = FILESYSTEM_ACCESS_UNKNOWN;
static ::sdk_util::SimpleLock s_mutex;

enum FilesystemHackType {
  FILESYSTEM_HACK_MAKE_DIRECTORY,
  FILESYSTEM_HACK_REMOVE,
  FILESYSTEM_HACK_WRITE,
};

struct FilesystemHackInfo {
  FilesystemHackType type;
  std::string path;
};

typedef std::deque<FilesystemHackInfo> InfoDeque;
static InfoDeque s_info_deque;

bool ReplacePath(const char* from, const char* to, std::string* path) {
  size_t ix = path->find(from);
  if (ix != 0) {
    return false;
  }
  path->replace(0, strlen(from), to);
  return true;
}

std::string PathJoin(const char* write_dir, const char* path) {
  return std::string(write_dir) + '/' + path;
}

std::string GetRealPath(const char* write_dir, const char* path) {
  std::string result(PathJoin(write_dir, path));
  if (ReplacePath(kFakePersistentDir, kRealPersistentDir, &result)) {
    return result;
  }
  return std::string();
}

void RequestFilesystemAccess() {
  if (s_filesystem_access == FILESYSTEM_ACCESS_UNKNOWN &&
      !s_requested_filesystem_access) {
    s_requested_filesystem_access = true;
    g_Instance->PostMessage(kRequestFilesystemMessage);
  }
}

bool MakeDirectoryInternal(const std::string& path) {
  size_t slash = path.find('/');
  for (int count = 0;
       slash != std::string::npos;
       count++, slash = path.find('/', slash + 1)) {
    if (count <= 1) {
      // Skip root "/" and mount point (e.g. "/persistent")
      continue;
    }

    mkdir(path.substr(0, slash).c_str(), 0777);
  }

  if (mkdir(path.c_str(), 0777) != 0) {
    fprintf(stderr, "mkdir(%s) failed with errno: %d\n", path.c_str(), errno);
    return false;
  }

  return true;
}

bool RemoveFileInternal(const std::string& path) {
  if (remove(path.c_str()) != 0) {
    fprintf(stderr, "remove(%s) failed with errno: %d\n", path.c_str(), errno);
    return false;
  }

  return true;
}

void PrintCopyFileError(FILE* file, const char* path, const char* op,
                        size_t op_bytes, size_t bytes_copied,
                        size_t total_bytes) {
  const char* error = "unknown";
  if (feof(file)) {
    error = "feof";
  } else if (ferror(file)) {
    error = "ferror";
  }
  fprintf(stderr, "CopyFile: %s(%s, %u) failed (%s). [%u/%u] bytes copied.\n",
          op, path, op_bytes, error, bytes_copied, total_bytes);
}

bool CopyFile(const char* src_path, const char* dst_path) {
  const size_t kMaxBufferSize = 32 * 1024;
  FILE* src = NULL;
  FILE* dst = NULL;
  uint8_t* buffer = NULL;
  bool result = false;
  struct stat statbuf;
  size_t size;
  size_t buffer_size;
  size_t bytes_left;

  if (stat(src_path, &statbuf) != 0) {
    fprintf(stderr, "stat(%s) failed with errno: %d\n", src_path, errno);
    return false;
  }

  size = statbuf.st_size;

  src = fopen(src_path, "r");
  if (!src) {
    fprintf(stderr, "fopen(%s) failed with errno: %d\n", src_path, errno);
    goto cleanup;
  }

  dst = fopen(dst_path, "w");
  if (!dst) {
    fprintf(stderr, "fopen(%s) failed with errno: %d\n", dst_path, errno);
    goto cleanup;
  }

  buffer_size = std::min(size, kMaxBufferSize);
  buffer = new uint8_t[buffer_size];

  bytes_left = size;
  while (bytes_left > 0) {
    size_t bytes_read = std::min(bytes_left, buffer_size);
    if (fread(buffer, bytes_read, 1, src) != 1) {
      PrintCopyFileError(src, src_path, "fread", bytes_read, size - bytes_left,
                         size);
      goto cleanup;
    }

    if (fwrite(buffer, bytes_read, 1, dst) != 1) {
      PrintCopyFileError(dst, dst_path, "fwrite", bytes_read, size - bytes_left,
                         size);
      goto cleanup;
    }

    bytes_left -= bytes_read;
  }

  result = true;
cleanup:
  delete[] buffer;
  if (dst) fclose(dst);
  if (src) fclose(src);
  return result;
}

bool CopyFileForWriteInternal(const std::string& src_path) {
  std::string dst_path(src_path);
  if (!ReplacePath(kFakePersistentDir, kRealPersistentDir, &dst_path)) {
    fprintf(stderr, "Strange path: %s\n", src_path.c_str());
    return false;
  }

  return CopyFile(src_path.c_str(), dst_path.c_str());
}

void ProcessInfoDeque() {
  for (InfoDeque::const_iterator iter = s_info_deque.begin();
       iter != s_info_deque.end();
       ++iter) {
    const FilesystemHackInfo& info = *iter;
    switch (info.type) {
      case FILESYSTEM_HACK_MAKE_DIRECTORY:
        MakeDirectoryInternal(info.path);
        break;
      case FILESYSTEM_HACK_REMOVE:
        RemoveFileInternal(info.path);
        break;
      case FILESYSTEM_HACK_WRITE:
        CopyFileForWriteInternal(info.path);
        break;
    }
  }

  s_info_deque.clear();
}

}  // namespace


void InitializeFilesystemHack() {
}

void SetFilesystemAccessAllowed(bool allowed) {
  AUTO_LOCK(s_mutex);
  if (s_filesystem_access != FILESYSTEM_ACCESS_UNKNOWN) {
    fprintf(stderr, "SetFilesystemAccessAllowed: called again.\n");
    return;
  }

  if (allowed) {
    s_filesystem_access = FILESYSTEM_ACCESS_ALLOWED;
    ProcessInfoDeque();
  } else {
    s_filesystem_access = FILESYSTEM_ACCESS_DISALLOWED;
    s_info_deque.clear();
  }
}

bool MakeDirectory(const char* writedir, const char* path) {
  std::string abs_path = GetRealPath(writedir, path);
  if (abs_path.empty()) {
    fprintf(stderr, "Strange path: %s\n", path);
    return false;
  }

  AUTO_LOCK(s_mutex);
  RequestFilesystemAccess();
  if (s_filesystem_access == FILESYSTEM_ACCESS_ALLOWED) {
    return MakeDirectoryInternal(abs_path);
  } else if (s_filesystem_access == FILESYSTEM_ACCESS_UNKNOWN) {
    // Queue up request.
    FilesystemHackInfo info;
    info.type = FILESYSTEM_HACK_MAKE_DIRECTORY;
    info.path = abs_path;
    s_info_deque.push_back(info);
  }

  return true;
}

bool RemoveFile(const char* writedir, const char* path) {
  std::string abs_path = GetRealPath(writedir, path);
  if (abs_path.empty()) {
    fprintf(stderr, "Strange path: %s\n", path);
    return false;
  }

  AUTO_LOCK(s_mutex);
  RequestFilesystemAccess();
  if (s_filesystem_access == FILESYSTEM_ACCESS_ALLOWED) {
    return RemoveFileInternal(abs_path);
  } else if (s_filesystem_access == FILESYSTEM_ACCESS_UNKNOWN) {
    // Queue up request.
    FilesystemHackInfo info;
    info.type = FILESYSTEM_HACK_REMOVE;
    info.path = abs_path;
    s_info_deque.push_back(info);
  }

  return true;
}

bool CopyFileForWrite(const char* writedir, const char* path) {
  std::string src_path = PathJoin(writedir, path);

  AUTO_LOCK(s_mutex);
  RequestFilesystemAccess();
  if (s_filesystem_access == FILESYSTEM_ACCESS_ALLOWED) {
    return CopyFileForWriteInternal(src_path);
  } else if (s_filesystem_access == FILESYSTEM_ACCESS_UNKNOWN) {
    // Queue up request.
    FilesystemHackInfo info;
    info.type = FILESYSTEM_HACK_WRITE;
    info.path = src_path;
    s_info_deque.push_back(info);
  }

  return true;
}

bool MakeDirectoryForRead(const char* path) {
  std::string dst_path = std::string(kFakePersistentDir) + path;
  return MakeDirectoryInternal(dst_path);
}

bool CopyFileForRead(const char* path) {
  std::string src_path = std::string(kRealPersistentDir) + path;
  std::string dst_path = std::string(kFakePersistentDir) + path;
  return CopyFile(src_path.c_str(), dst_path.c_str());
}

}  // ppapi
}  // window
}  // love
