/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "GameInfoLoader.h"
#include "log/Log.h"

#include <kodi/Filesystem.h>
#include <kodi/General.h>

#include <algorithm>
#include <new>
#include <stdint.h>

using namespace LIBRETRO;

#define READ_SIZE      (100 * 1024)         // Read from VFS 100KB at a time (if file size is unknown)
#define MAX_READ_SIZE  (513 * 1024 * 1024)  // Read at most the largest Nintendo DS cartridge, 512MB, and a header
#define MAX_UNCHECKED_READ_SIZE (100 * 1024 * 1024)  // Read at most 100MB when size or free memory is unknown

namespace
{
// The local file a path names, or empty for a path only the VFS can open
std::string LocalFile(const std::string& path)
{
  const std::string fileProtocol = "file://";
  if (path.compare(0, fileProtocol.size(), fileProtocol) == 0)
    return path.substr(fileProtocol.size());
  if (path.find("://") != std::string::npos)
    return "";
  return path;
}
} // namespace

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
  // whole file is loaded, within limits. Loading by path is the fallback
  // whenever it can't be.
  try
  {
    uint64_t size = statStruct.GetSize();
    if (size > MAX_READ_SIZE)
    {
      dsyslog("File size (%d MB) is greater than memory limit (%d MB), loading by path",
              static_cast<int>(size / (1024 * 1024)), MAX_READ_SIZE / (1024 * 1024));
      return false;
    }
    else if (size > 0)
    {
      // An allocation the system can't back gets Kodi killed rather than
      // throwing, so only load what fits in the free memory twice over with
      // some to spare, as cores like melonDS make their own copy
      long freeMB = 0;
      long totalMB = 0;
      kodi::GetFreeMem(freeMB, totalMB);
      const uint64_t sizeMB = size / (1024 * 1024);
      if (totalMB <= 0)
      {
        // Kodi couldn't tell, so keep to the limit there was before
        if (size > MAX_UNCHECKED_READ_SIZE)
        {
          esyslog("Free memory unknown, not loading a %d MB file into memory, loading by path: %s",
                  static_cast<int>(sizeMB), m_path.c_str());
          return false;
        }
      }
      else if (sizeMB * 2 + 128 > static_cast<uint64_t>(std::max(freeMB, 0L)))
      {
        esyslog("Not enough free memory (%ld MB) to load a %d MB file, loading by path: %s",
                freeMB, static_cast<int>(sizeMB), m_path.c_str());
        return false;
      }

      // A local file is mapped rather than read, so it isn't copied
      const std::string localFile = LocalFile(m_path);
      if (!localFile.empty() && m_mapping.Map(localFile))
      {
        dsyslog("Mapped file into memory (%zu bytes): %s", m_mapping.Size(), m_path.c_str());
        return true;
      }

      // Size is known, read entire file at once
      m_dataBuffer.resize(static_cast<size_t>(size));
      size_t total = 0;
      ssize_t bytesRead;
      while (total < m_dataBuffer.size() &&
             (bytesRead = file.Read(m_dataBuffer.data() + total, m_dataBuffer.size() - total)) > 0)
        total += static_cast<size_t>(bytesRead);

      // A read that stops short would hand the core a truncated game
      if (total < m_dataBuffer.size())
      {
        std::vector<uint8_t>().swap(m_dataBuffer);
        esyslog("Read %d of %d MB, loading by path: %s", static_cast<int>(total / (1024 * 1024)),
                static_cast<int>(sizeMB), m_path.c_str());
        return false;
      }
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

        // The buffer grows as it reads, and each reallocation briefly needs
        // more, so a file of unknown size keeps a lower limit
        if (m_dataBuffer.size() > MAX_UNCHECKED_READ_SIZE)
        {
          std::vector<uint8_t>().swap(m_dataBuffer);
          dsyslog("File exceeds memory limit (%d MB), loading by path",
                  MAX_UNCHECKED_READ_SIZE / (1024 * 1024));
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
  const bool bMapped = m_mapping.Data() != nullptr;
  if (bMapped || !m_dataBuffer.empty())
  {
    //! @todo path is null according to libretro API, but many cores expect
    //        the frontend to set this. Do so to guard against
    //        noncompliant cores.
    info.path = m_path.c_str();

    info.data = bMapped ? m_mapping.Data() : m_dataBuffer.data();
    info.size = bMapped ? m_mapping.Size() : m_dataBuffer.size();
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
