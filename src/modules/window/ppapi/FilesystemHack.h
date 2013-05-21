#ifndef LOVE_WINDOW_PPAPI_FILESYSTEM_HACK_H_
#define LOVE_WINDOW_PPAPI_FILESYSTEM_HACK_H_

namespace love {
namespace window {
namespace ppapi {

void InitializeFilesystemHack();
void SetFilesystemAccessAllowed(bool allowed);
bool MakeDirectory(const char* writedir, const char* path);
bool RemoveFile(const char* writedir, const char* path);
bool CopyFileForWrite(const char* writedir, const char* path);

bool MakeDirectoryForRead(const char* path);
bool CopyFileForRead(const char* path);

}  // namespace ppapi
}  // namespace window
}  // namespace love

#endif  // LOVE_WINDOW_PPAPI_FILESYSTEM_HACK_H_
