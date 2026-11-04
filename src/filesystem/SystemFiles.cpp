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
#include "utils/Hash.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>

#include <kodi/Filesystem.h>

using namespace LIBRETRO;

bool CSystemFiles::Copy(const std::vector<std::string>& resourceDirectories,
                        const std::string& systemDirectory,
                        const std::string& ownDirectory,
                        const std::string& version,
                        const std::string& recordPath)
{
  std::map<std::string, kodi::vfs::CDirEntry> files;
  for (const std::string& directory : resourceDirectories)
  {
    const std::string system = CFilesystemUtils::CoreDirectory(
        CFilesystemUtils::AddFileToFolder(directory, LIBRETRO_SYSTEM_DIRECTORY_NAME));
    // Most emulators ship none, and Kodi logs an error for a missing folder
    if (!CFilesystemUtils::SameFolder(system, systemDirectory) &&
        !CFilesystemUtils::SameFolder(system, ownDirectory) && kodi::vfs::DirectoryExists(system))
      CFilesystemUtils::ListFiles(system, files);
  }

  if (files.empty())
    return false;

  // The folder they go into, the emulator's version, and its files' names,
  // sizes and times
  uint64_t fingerprint = HashBytes(systemDirectory.c_str(), systemDirectory.size() + 1);
  fingerprint = HashBytes(version.c_str(), version.size() + 1, fingerprint);
  for (auto& [name, file] : files)
  {
    const int64_t size = file.Size();
    const auto time = static_cast<int64_t>(file.DateTime());
    fingerprint = HashBytes(name.c_str(), name.size() + 1, fingerprint);
    fingerprint = HashBytes(&size, sizeof(size), fingerprint);
    fingerprint = HashBytes(&time, sizeof(time), fingerprint);
  }
  const std::string record = std::to_string(fingerprint);

  // Nothing has changed since the last copy, and every file is still there
  std::string text;
  if (CFilesystemUtils::ReadText(recordPath, text) && text == record &&
      std::all_of(files.begin(), files.end(),
                  [&systemDirectory](const auto& file)
                  {
                    return kodi::vfs::FileExists(
                        CFilesystemUtils::AddFileToFolder(systemDirectory, file.first));
                  }))
    return true;

  if (!kodi::vfs::DirectoryExists(systemDirectory) && !kodi::vfs::CreateDirectory(systemDirectory))
  {
    esyslog("Failed to create system folder %s", systemDirectory.c_str());
    return false;
  }

  unsigned int copied = 0;
  for (auto& [name, file] : files)
  {
    const std::string target = CFilesystemUtils::AddFileToFolder(systemDirectory, name);
    if (CFilesystemUtils::CreateFolders(systemDirectory, name) &&
        CFilesystemUtils::CopyInto(file.Path(), target))
      ++copied;
    else
      esyslog("Failed to copy system file %s to %s", file.Path().c_str(), target.c_str());
  }

  // A file that couldn't be copied is tried again the next time
  const bool complete = copied == files.size();
  if (!CFilesystemUtils::WriteText(recordPath, complete ? record : std::string()))
    esyslog("Failed to write %s", recordPath.c_str());

  isyslog("System folder %s: copied %u of %u system files", systemDirectory.c_str(), copied,
          static_cast<unsigned int>(files.size()));
  return complete;
}
