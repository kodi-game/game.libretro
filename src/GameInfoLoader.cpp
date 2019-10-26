/*
 *      Copyright (C) 2014-2016 Team Kodi
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

#include "GameInfoLoader.h"
#include "log/Log.h"

#include <kodi/Filesystem.h>

#include <stdint.h>

using namespace LIBRETRO;

#define READ_SIZE      (100 * 1024)         // Read from VFS 100KB at a time (if file size is unknown)
#define MAX_READ_SIZE  (100 * 1024 * 1024)  // Read at most 100MB from VFS

CGameInfoLoader::CGameInfoLoader(const std::string& path, bool bSupportsVFS)
 : m_path(path),
   m_bSupportsVfs(bSupportsVFS)
{
}

bool CGameInfoLoader::Load(void)
{
  if (!m_bSupportsVfs)
    return false;

  STAT_STRUCTURE statStruct = {0};
  bool bExists = kodi::vfs::StatFile(m_path, statStruct);

  // Not all VFS protocols necessarily support StatFile(), so also check if file exists
  if (!bExists)
  {
    bExists = kodi::vfs::FileExists(m_path, true);
    if (bExists)
    {
      dsyslog("Failed to stat (but file exists): %s", m_path.c_str());
    }
    else
    {
      esyslog("File doesn't exist: %s", m_path.c_str());
      return false;
    }
  }

  kodi::vfs::CFile file;
  if (!file.OpenFile(m_path))
  {
    esyslog("Failed to open file: %s", m_path.c_str());
    return false;
  }

  int64_t size = statStruct.size;
  if (size > 0)
  {
    // Size is known, read entire file at once (unless it is too big)
    if (size <= MAX_READ_SIZE)
    {
      m_dataBuffer.resize((size_t)size);
      file.Read(m_dataBuffer.data(), size);
    }
    else
    {
      dsyslog("File size (%d MB) is greater than memory limit (%d MB), loading by path",
              size / (1024 * 1024), MAX_READ_SIZE / (1024 * 1024));
      return false;
    }
  }
  else
  {
    // Read file in chunks
    unsigned int bytesRead;
    uint8_t buffer[READ_SIZE];
    while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
    {
      m_dataBuffer.insert(m_dataBuffer.end(), buffer, buffer + bytesRead);

      // If we read less than READ_SIZE, assume we hit the end of the file
      if (bytesRead < READ_SIZE)
        break;

      // If we have exceeded the VFS file size limit, don't try to load by
      // VFS and fall back to loading by path
      if (m_dataBuffer.size() > MAX_READ_SIZE)
      {
        dsyslog("File exceeds memory limit (%d MB), loading by path",
                MAX_READ_SIZE / (1024 * 1024));
        return false;
      }
    }
  }

  if (m_dataBuffer.empty())
  {
    dsyslog("Failed to read file (no data), loading by path");
    return false;
  }

  dsyslog("Loaded file into memory (%d bytes): %s", m_dataBuffer.size(), m_path.c_str());

  return true;
}

bool CGameInfoLoader::GetMemoryStruct(retro_game_info& info) const
{
  if (!m_dataBuffer.empty())
  {
    //! @todo path is null according to libretro API, but many cores expect
    //        the frontend to set this. Do so to guard against
    //        noncompliant cores.
    info.path = m_path.c_str();

    info.data = m_dataBuffer.data();
    info.size = m_dataBuffer.size();
    info.meta = nullptr;
    return true;
  }
  return false;
}

bool CGameInfoLoader::GetPathStruct(retro_game_info& info) const
{
  info.path = m_path.c_str();
  info.data = nullptr;
  info.size = 0;
  info.meta = nullptr;
  return true;
}
