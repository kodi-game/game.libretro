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
