/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroResources.h"
#include "LibretroDefines.h"
#include "log/Log.h"

#include "client.h"

#include <kodi/Filesystem.h>
#include <assert.h>
#include <utility>

using namespace LIBRETRO;

CLibretroResources::CLibretroResources() :
  m_addon(nullptr)
{
}

void CLibretroResources::Initialize(CGameLibRetro* addon)
{
  m_addon = addon;

  assert(m_addon != nullptr);

  std::vector<std::string> dirs;
  m_addon->ResourceDirectories(dirs);
  for (const auto& dir : dirs)
  {
    if (dir.empty())
      continue;

    // Set system path to first resource path discovered
    if (m_systemDirectory.empty())
    {
      m_systemDirectory = dir + "/" LIBRETRO_SYSTEM_DIRECTORY_NAME;

      // Ensure folder exists
      if (!kodi::vfs::DirectoryExists(m_systemDirectory))
      {
        dsyslog("Creating system directory: %s", m_systemDirectory.c_str());
        kodi::vfs::CreateDirectory(m_systemDirectory);
      }
    }

  m_resourceDirectories.push_back(std::move(dir));
}

  m_saveDirectory = m_addon->ProfileDirectory() + "/" LIBRETRO_SAVE_DIRECTORY_NAME;

  // Ensure folder exists
  if (!kodi::vfs::DirectoryExists(m_saveDirectory))
  {
    dsyslog("Creating save directory: %s", m_saveDirectory.c_str());
    kodi::vfs::CreateDirectory(m_saveDirectory);
  }
}

void CLibretroResources::Deinitialize()
{
  m_addon = nullptr;
}

const char* CLibretroResources::GetBasePath(const std::string& relPath)
{
  auto it = m_pathMap.find(relPath);

  if (it == m_pathMap.end())
  {
    for (const auto& dir : m_resourceDirectories)
    {
      std::string resourcePath = dir + "/" + relPath;

      // Check for path existence
      if (kodi::vfs::FileExists(resourcePath, true))
      {
        m_pathMap.insert(std::make_pair(relPath, std::move(dir)));
        it = m_pathMap.find(relPath);
        break;
      }
    }
  }

  if (it != m_pathMap.end())
    return it->second.c_str();

  return nullptr;
}

const char* CLibretroResources::GetBaseSystemPath(const std::string& relPath)
{
  std::string systemPath = LIBRETRO_SYSTEM_DIRECTORY_NAME "/" + relPath;
  const char* basePath = GetBasePath(systemPath);
  if (basePath != nullptr)
    return ApendSystemFolder(basePath);

  return nullptr;
}

std::string CLibretroResources::CLibretroResources::GetFullPath(const std::string& relPath)
{
  const char* basePath = GetBasePath(relPath);

  if (basePath != nullptr)
    return std::string(basePath) + "/" + relPath;

  return "";
}

std::string CLibretroResources::GetFullSystemPath(const std::string& relPath)
{
  const char* baseSystemPath = GetBaseSystemPath(relPath);

  if (baseSystemPath != nullptr)
    return std::string(baseSystemPath) + "/" + relPath;

  return "";
}

const char* CLibretroResources::ApendSystemFolder(const std::string& path)
{
  static std::map<std::string, std::string> pathCache;

  auto it = pathCache.find(path);
  if (it == pathCache.end())
  {
    const std::string systemPath = path + "/" LIBRETRO_SYSTEM_DIRECTORY_NAME;

    pathCache.insert(std::make_pair(path, std::move(systemPath)));
    it = pathCache.find(path);
  }

  if (it != pathCache.end())
    return it->second.c_str();

  return nullptr;

}
