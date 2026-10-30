/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FilesystemUtils.h"

#include <algorithm>
#include <vector>

using namespace LIBRETRO;

namespace
{
void ListFilesIn(const std::string& folder,
                 const std::string& prefix,
                 std::map<std::string, kodi::vfs::CDirEntry>& files)
{
  std::vector<kodi::vfs::CDirEntry> items;
  if (!kodi::vfs::GetDirectory(folder, "", items))
    return;

  for (auto& item : items)
  {
    const std::string name = prefix.empty() ? item.Label() : prefix + "/" + item.Label();
    if (item.IsFolder())
      ListFilesIn(item.Path(), name, files);
    else
      files.emplace(name, item);
  }
}
} // namespace

std::string CFilesystemUtils::CoreDirectory(std::string path)
{
  while (path.size() > 1 && (path.back() == '/' || path.back() == '\\') &&
         !(path.size() == 3 && path[1] == ':'))
    path.pop_back();

  return path;
}

std::string CFilesystemUtils::AddFileToFolder(const std::string& folder, const std::string& file)
{
  if (!folder.empty() && (folder.back() == '/' || folder.back() == '\\'))
    return folder + file;
  return folder + "/" + file;
}

bool CFilesystemUtils::SameFolder(std::string a, std::string b)
{
  std::replace(a.begin(), a.end(), '\\', '/');
  std::replace(b.begin(), b.end(), '\\', '/');
  return a == b;
}

void CFilesystemUtils::ListFiles(const std::string& folder,
                                 std::map<std::string, kodi::vfs::CDirEntry>& files)
{
  ListFilesIn(folder, "", files);
}

bool CFilesystemUtils::CreateFolders(const std::string& folder, const std::string& path)
{
  for (size_t separator = path.find('/'); separator != std::string::npos;
       separator = path.find('/', separator + 1))
  {
    const std::string parent = AddFileToFolder(folder, path.substr(0, separator));
    if (!kodi::vfs::DirectoryExists(parent) && !kodi::vfs::CreateDirectory(parent))
      return false;
  }
  return true;
}

bool CFilesystemUtils::CopyInto(const std::string& source, const std::string& target)
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

bool CFilesystemUtils::ReadText(const std::string& path, std::string& text)
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

bool CFilesystemUtils::WriteText(const std::string& path, const std::string& text)
{
  kodi::vfs::CFile file;
  return file.OpenFileForWrite(path, true) &&
         file.Write(text.data(), text.size()) == static_cast<ssize_t>(text.size());
}
