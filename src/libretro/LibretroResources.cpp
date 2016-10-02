/*
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "LibretroResources.h"
#include "LibretroDefines.h"

#include "kodi/kodi_game_types.h"
#include "kodi/libXBMC_addon.h"

#include <utility>

using namespace LIBRETRO;

namespace LIBRETRO
{
  // Trailing slash causes some libretro cores to fail
  void RemoveSlashAtEnd(std::string& path)
  {
    if (!path.empty())
    {
      char last = path[path.size() - 1];
      if (last == '/' || last == '\\')
        path.erase(path.size() - 1);
    }
  }
}

void CLibretroResources::Initialize(ADDON::CHelper_libXBMC_addon* addon, const game_client_properties* gameClientProps)
{
  for (unsigned int i = 0; i < gameClientProps->resource_directory_count; i++)
  {
    if (gameClientProps->resource_directories[i] == nullptr)
      break;

    std::string resourcePath = gameClientProps->resource_directories[i];

    RemoveSlashAtEnd(resourcePath);

    // Set system path to first resource path discovered
    if (m_systemDirectory.empty())
      m_systemDirectory = resourcePath + "/" LIBRETRO_SYSTEM_DIRECTORY_NAME;

    m_resourceDirectories.push_back(std::move(resourcePath));
  }

  if (gameClientProps->profile_directory != nullptr)
  {
    m_saveDirectory = gameClientProps->profile_directory;
    RemoveSlashAtEnd(m_saveDirectory);
    m_saveDirectory += "/" LIBRETRO_SAVE_DIRECTORY_NAME;

    // Ensure folder exists
    if (addon)
    {
      if (!addon->DirectoryExists(m_saveDirectory.c_str()))
      {
        addon->Log(ADDON::LOG_DEBUG, "Creating save directory: %s", m_saveDirectory.c_str());
        addon->CreateDirectory(m_saveDirectory.c_str());
      }
    }
  }
}

const char* CLibretroResources::GetBasePath(const std::string& relPath)
{
  auto it = m_pathMap.find(relPath);

  if (it == m_pathMap.end())
  {
    for (const auto& dir : m_resourceDirectories)
    {
      std::string resourcePath = dir + relPath; // TODO: Check for path existence

      if (!dir.empty())
      {
        m_pathMap.insert(std::make_pair(relPath, std::move(dir)));
        it = m_pathMap.find(relPath);
      }
      break;
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
