/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SystemFiles.h"

#include "FilesystemDefines.h"
#include "FilesystemUtils.h"
#include "log/Log.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>

#include <kodi/Filesystem.h>

using namespace LIBRETRO;

namespace
{
struct SystemFile
{
  std::string path;
  int64_t size;
  int64_t time;
};

std::string Join(const std::string& folder, const std::string& name)
{
  if (!folder.empty() && (folder.back() == '/' || folder.back() == '\\'))
    return folder + name;
  return folder + "/" + name;
}

// Every file in a folder and the folders in it, by its path inside the folder.
// A path already listed, from a folder that takes precedence, is kept.
void ListFiles(const std::string& folder,
               const std::string& prefix,
               std::map<std::string, SystemFile>& files)
{
  std::vector<kodi::vfs::CDirEntry> items;
  if (!kodi::vfs::GetDirectory(folder, "", items))
    return;

  for (auto& item : items)
  {
    const std::string name = prefix.empty() ? item.Label() : prefix + "/" + item.Label();
    if (item.IsFolder())
      ListFiles(item.Path(), name, files);
    else
      files.emplace(name,
                    SystemFile{item.Path(), item.Size(), static_cast<int64_t>(item.DateTime())});
  }
}

// 64-bit FNV-1a
uint64_t HashBytes(const void* data, size_t size, uint64_t hash = 14695981039346656037ULL)
{
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (size_t i = 0; i < size; ++i)
  {
    hash ^= bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

// Kodi separates a Windows path's folders with backslashes
bool SameFolder(std::string a, std::string b)
{
  std::replace(a.begin(), a.end(), '\\', '/');
  std::replace(b.begin(), b.end(), '\\', '/');
  return a == b;
}

// The folders a file's path inside the system folder runs through
bool CreateFolders(const std::string& systemDirectory, const std::string& name)
{
  for (size_t separator = name.find('/'); separator != std::string::npos;
       separator = name.find('/', separator + 1))
  {
    const std::string folder = Join(systemDirectory, name.substr(0, separator));
    if (!kodi::vfs::DirectoryExists(folder) && !kodi::vfs::CreateDirectory(folder))
      return false;
  }
  return true;
}

// The copy is made beside the target and moved over it, so a file copied only
// partly never takes its place, and the file it replaces is only removed once
// the copy is in place
bool CopyInto(const std::string& source, const std::string& target)
{
  const std::string partial = target + ".partial";
  if (!kodi::vfs::CopyFile(source, partial))
  {
    kodi::vfs::DeleteFile(partial);
    return false;
  }
  if (kodi::vfs::RenameFile(partial, target))
    return true;

  // Some filesystems can't rename over a file, so the old one is moved aside
  const std::string previous = target + ".previous";
  if (kodi::vfs::FileExists(target) && kodi::vfs::RenameFile(target, previous))
  {
    if (kodi::vfs::RenameFile(partial, target))
    {
      kodi::vfs::DeleteFile(previous);
      return true;
    }
    kodi::vfs::RenameFile(previous, target);
  }
  kodi::vfs::DeleteFile(partial);
  return false;
}

bool ReadText(const std::string& path, std::string& text)
{
  kodi::vfs::CFile file;
  if (!kodi::vfs::FileExists(path) || !file.OpenFile(path))
    return false;

  char buffer[4096];
  ssize_t bytes;
  while ((bytes = file.Read(buffer, sizeof(buffer))) > 0)
    text.append(buffer, static_cast<size_t>(bytes));
  return bytes == 0;
}

bool WriteText(const std::string& path, const std::string& text)
{
  kodi::vfs::CFile file;
  return file.OpenFileForWrite(path, true) &&
         file.Write(text.data(), text.size()) == static_cast<ssize_t>(text.size());
}
} // namespace

bool CSystemFiles::Copy(const std::vector<std::string>& resourceDirectories,
                        const std::string& systemDirectory,
                        const std::string& ownDirectory,
                        const std::string& version,
                        const std::string& recordPath)
{
  std::map<std::string, SystemFile> files;
  for (const std::string& directory : resourceDirectories)
  {
    const std::string system =
        CFilesystemUtils::CoreDirectory(Join(directory, LIBRETRO_SYSTEM_DIRECTORY_NAME));
    // Most emulators ship none, and Kodi logs an error for a missing folder
    if (!SameFolder(system, systemDirectory) && !SameFolder(system, ownDirectory) &&
        kodi::vfs::DirectoryExists(system))
      ListFiles(system, "", files);
  }

  if (files.empty())
    return false;

  // The folder they go into, the emulator's version, and its files' names,
  // sizes and times
  uint64_t fingerprint = HashBytes(systemDirectory.c_str(), systemDirectory.size() + 1);
  fingerprint = HashBytes(version.c_str(), version.size() + 1, fingerprint);
  for (const auto& [name, file] : files)
  {
    fingerprint = HashBytes(name.c_str(), name.size() + 1, fingerprint);
    fingerprint = HashBytes(&file.size, sizeof(file.size), fingerprint);
    fingerprint = HashBytes(&file.time, sizeof(file.time), fingerprint);
  }
  const std::string record = std::to_string(fingerprint);

  // Nothing has changed since the last copy, and every file is still there
  std::string text;
  if (ReadText(recordPath, text) && text == record &&
      std::all_of(files.begin(), files.end(), [&systemDirectory](const auto& file)
                  { return kodi::vfs::FileExists(Join(systemDirectory, file.first)); }))
    return true;

  if (!kodi::vfs::DirectoryExists(systemDirectory) && !kodi::vfs::CreateDirectory(systemDirectory))
  {
    esyslog("Failed to create system folder %s", systemDirectory.c_str());
    return false;
  }

  unsigned int copied = 0;
  for (const auto& [name, file] : files)
  {
    const std::string target = Join(systemDirectory, name);
    if (CreateFolders(systemDirectory, name) && CopyInto(file.path, target))
      ++copied;
    else
      esyslog("Failed to copy system file %s to %s", file.path.c_str(), target.c_str());
  }

  // A file that couldn't be copied is tried again the next time
  const bool complete = copied == files.size();
  if (!WriteText(recordPath, complete ? record : std::string()))
    esyslog("Failed to write %s", recordPath.c_str());

  isyslog("System folder %s: copied %u of %u system files", systemDirectory.c_str(), copied,
          static_cast<unsigned int>(files.size()));
  return complete;
}
