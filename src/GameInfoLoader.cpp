/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "GameInfoLoader.h"
#include "log/Log.h"

#include <kodi/Filesystem.h>

#include <new>
#include <stdint.h>

using namespace LIBRETRO;

#define READ_SIZE      (100 * 1024)         // Read from VFS 100KB at a time (if file size is unknown)
#define MAX_READ_SIZE  (512 * 1024 * 1024)  // Read at most 512MB, the largest Nintendo DS cartridge

CGameInfoLoader::CGameInfoLoader(const std::string& path, bool bSupportsVFS)
 : m_path(path),
   m_bSupportsVfs(bSupportsVFS)
{
}

bool CGameInfoLoader::Load(void)
{
  if (!m_bSupportsVfs)
    return false;

  kodi::vfs::FileStatus statStruct;
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

  // A core that doesn't need a full path reads the game from memory, so the
  // whole file is loaded, up to a limit. Loading by path is only a fallback for
  // a bigger file or when the memory can't be had.
  try
  {
    uint64_t size = statStruct.GetSize();
    if (size > MAX_READ_SIZE)
    {
      dsyslog("File size (%d MB) is greater than memory limit (%d MB), loading by path",
              size / (1024 * 1024), MAX_READ_SIZE / (1024 * 1024));
      return false;
    }
    else if (size > 0)
    {
      // Size is known, read entire file at once
      m_dataBuffer.resize(static_cast<size_t>(size));
      size_t total = 0;
      ssize_t bytesRead;
      while (total < m_dataBuffer.size() &&
             (bytesRead = file.Read(m_dataBuffer.data() + total, m_dataBuffer.size() - total)) > 0)
        total += static_cast<size_t>(bytesRead);
      m_dataBuffer.resize(total);
    }
    else
    {
      // Read file in chunks
      ssize_t bytesRead;
      uint8_t buffer[READ_SIZE];
      while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
      {
        m_dataBuffer.insert(m_dataBuffer.end(), buffer, buffer + bytesRead);

        // If we read less than READ_SIZE, assume we hit the end of the file
        if (bytesRead < READ_SIZE)
          break;

        if (m_dataBuffer.size() > MAX_READ_SIZE)
        {
          std::vector<uint8_t>().swap(m_dataBuffer);
          dsyslog("File exceeds memory limit (%d MB), loading by path",
                  MAX_READ_SIZE / (1024 * 1024));
          return false;
        }
      }
    }
  }
  catch (const std::bad_alloc&)
  {
    std::vector<uint8_t>().swap(m_dataBuffer);
    esyslog("Not enough memory to load the file, loading by path: %s", m_path.c_str());
    return false;
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
