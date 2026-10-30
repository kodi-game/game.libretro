/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SharedSystemFiles.h"

#include "FilesystemDefines.h"
#include "FilesystemUtils.h"
#include "SharedSystemRecord.h"
#include "log/Log.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>

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
    // The record has a line for each path
    else if (name.find_first_of("\r\n") == std::string::npos)
      files.emplace(name,
                    SystemFile{item.Path(), item.Size(), static_cast<int64_t>(item.DateTime())});
  }
}

bool HashFile(const std::string& path, uint64_t& hash)
{
  kodi::vfs::CFile file;
  if (!file.OpenFile(path))
    return false;

  hash = HASH_START;
  std::vector<uint8_t> buffer(64 * 1024);
  ssize_t bytes;
  while ((bytes = file.Read(buffer.data(), buffer.size())) > 0)
    hash = HashBytes(buffer.data(), static_cast<size_t>(bytes), hash);
  return bytes == 0;
}

// The folders a file's path inside the shared folder runs through
bool CreateFolders(const std::string& sharedDirectory, const std::string& name)
{
  for (size_t separator = name.find('/'); separator != std::string::npos;
       separator = name.find('/', separator + 1))
  {
    const std::string folder = Join(sharedDirectory, name.substr(0, separator));
    if (!kodi::vfs::DirectoryExists(folder) && !kodi::vfs::CreateDirectory(folder))
      return false;
  }
  return true;
}

// The copy is made beside the target and moved over it, so a file copied only
// partly never takes its place
bool CopyInto(const std::string& source, const std::string& target)
{
  const std::string partial = target + ".partial";
  if (!kodi::vfs::CopyFile(source, partial))
  {
    kodi::vfs::DeleteFile(partial);
    return false;
  }
  if (kodi::vfs::FileExists(target) && !kodi::vfs::DeleteFile(target))
  {
    kodi::vfs::DeleteFile(partial);
    return false;
  }
  return kodi::vfs::RenameFile(partial, target);
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

void CSharedSystemFiles::Copy(const std::vector<std::string>& resourceDirectories,
                              const std::string& sharedDirectory,
                              const std::string& version,
                              const std::string& recordPath)
{
  std::map<std::string, SystemFile> files;
  for (const std::string& directory : resourceDirectories)
  {
    const std::string system =
        CFilesystemUtils::CoreDirectory(Join(directory, LIBRETRO_SYSTEM_DIRECTORY_NAME));
    // Most emulators ship none, and Kodi logs an error for a missing folder
    if (system != sharedDirectory && kodi::vfs::DirectoryExists(system))
      ListFiles(system, "", files);
  }

  if (files.empty())
    return;

  uint64_t fingerprint = HashBytes(version.data(), version.size());
  for (const auto& [name, file] : files)
  {
    fingerprint = HashBytes(name.c_str(), name.size() + 1, fingerprint);
    fingerprint = HashBytes(&file.size, sizeof(file.size), fingerprint);
    fingerprint = HashBytes(&file.time, sizeof(file.time), fingerprint);
  }

  CSharedSystemRecord record;
  std::string text;
  if (!ReadText(recordPath, text) || !record.Parse(text) ||
      record.sharedDirectory != sharedDirectory)
    record = CSharedSystemRecord();

  // Nothing has changed since the last time, and every file is still there
  if (record.fingerprint == fingerprint &&
      std::all_of(files.begin(), files.end(), [&sharedDirectory](const auto& file)
                  { return kodi::vfs::FileExists(Join(sharedDirectory, file.first)); }))
    return;

  unsigned int copied = 0;
  unsigned int replaced = 0;
  unsigned int kept = 0;
  bool failed = false;
  std::map<std::string, uint64_t> copies;

  for (const auto& [name, file] : files)
  {
    uint64_t source = 0;
    if (!HashFile(file.path, source))
    {
      esyslog("Failed to read system file %s", file.path.c_str());
      failed = true;
      continue;
    }

    const std::string target = Join(sharedDirectory, name);
    const bool exists = kodi::vfs::FileExists(target);
    uint64_t current = 0;
    if (exists && !HashFile(target, current))
    {
      ++kept;
      continue;
    }

    const auto earlier = record.copies.find(name);
    const uint64_t* earlierHash = earlier != record.copies.end() ? &earlier->second : nullptr;

    switch (DecideSharedFile(exists ? &current : nullptr, source, earlierHash))
    {
      case SharedFileAction::COPY:
      case SharedFileAction::REPLACE:
        if (CreateFolders(sharedDirectory, name) && CopyInto(file.path, target))
        {
          copies[name] = source;
          ++(exists ? replaced : copied);
        }
        else
        {
          esyslog("Failed to copy system file %s to %s", file.path.c_str(), target.c_str());
          failed = true;
          // Still the earlier copy, to replace next time
          if (exists)
            copies[name] = *earlierHash;
        }
        break;
      case SharedFileAction::NONE:
        copies[name] = source;
        break;
      case SharedFileAction::KEEP:
        ++kept;
        break;
    }
  }

  // A file that couldn't be copied is tried again the next time
  record.sharedDirectory = sharedDirectory;
  record.fingerprint = failed ? 0 : fingerprint;
  record.copies = std::move(copies);
  if (!WriteText(recordPath, record.Format()))
    esyslog("Failed to write %s", recordPath.c_str());

  if (copied > 0 || replaced > 0)
    isyslog("Shared system folder %s: copied %u files, updated %u", sharedDirectory.c_str(), copied,
            replaced);
  if (kept > 0)
    isyslog("Shared system folder %s: left %u files put there or changed by someone else",
            sharedDirectory.c_str(), kept);
}
