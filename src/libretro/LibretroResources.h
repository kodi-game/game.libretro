/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

class CGameLibRetro;

namespace LIBRETRO
{
  class CLibretroResources
  {
  public:
    CLibretroResources();
    ~CLibretroResources() { Deinitialize(); }

    void Initialize(CGameLibRetro* addon);
    void Deinitialize();

    const char* GetSystemDir() const { return m_systemDirectory.c_str(); }
    const char* GetContentDirectory() { return GetSystemDir(); } // Use system directory
    const char* GetSaveDirectory() const { return m_saveDirectory.c_str(); }

    const char* GetBasePath(const std::string& relPath);
    const char* GetBaseSystemPath(const std::string& relPath);
    std::string GetFullPath(const std::string& relPath);
    std::string GetFullSystemPath(const std::string& relPath);

  private:
    const char* ApendSystemFolder(const std::string& path);

    CGameLibRetro*                     m_addon;
    std::vector<std::string>           m_resourceDirectories;
    std::map<std::string, std::string> m_pathMap;
    std::string                        m_systemDirectory;
    std::string                        m_saveDirectory;
  };
} // namespace LIBRETRO
