/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GameInfoLoader.h"

// TODO: This must #defined before libXBMC_addon.h to fix compile on OS X
#include <sys/stat.h>
#if defined(TARGET_OSX)
  #define stat64 stat
  #define __stat64 stat
#elif !defined(TARGET_WINDOWS)
  #define __stat64 stat64
#endif

#include "kodi/libXBMC_addon.h"

#include <stdint.h>

using namespace ADDON;
using namespace LIBRETRO;

#define READ_SIZE      (100 * 1024)         // Read from VFS 100KB at a time (if file size is unknown)
#define MAX_READ_SIZE  (100 * 1024 * 1024)  // Read at most 100MB from VFS

CGameInfoLoader::CGameInfoLoader(const char* path, CHelper_libXBMC_addon* XBMC, bool bSupportsVFS)
 : m_pathBuffer(path)
{
  // If libretro client supports loading from memory, try reading the file into m_dataBuffer
  if (bSupportsVFS)
  {
    struct __stat64 statStruct = { };

    // Not all VFS protocols necessarily support StatFile(), so also check if file exists
    if (XBMC->StatFile(path, &statStruct) == 0 || XBMC->FileExists(path, true))
    {
      void* file = XBMC->OpenFile(path, 0);
      if (file)
      {
        int64_t size = statStruct.st_size;
        if (size > 0)
        {
          // Size is known, read entire file at once (unless it is too big)
          if (size <= MAX_READ_SIZE)
          {
            m_dataBuffer.resize((size_t)size);
            XBMC->ReadFile(file, m_dataBuffer.data(), size);
          }
        }
        else
        {
          // Read file in chunks
          unsigned int bytesRead;
          uint8_t buffer[READ_SIZE];
          while ((bytesRead = XBMC->ReadFile(file, buffer, sizeof(buffer))) > 0)
          {
            m_dataBuffer.insert(m_dataBuffer.end(), buffer, buffer + bytesRead);

            // If we read less than READ_SIZE, assume we hit the end of the file
            if (bytesRead < READ_SIZE)
              break;

            // If we have exceeded the VFS file size limit, don't try to load by
            // VFS and fall back to loading by path
            if (m_dataBuffer.size() > MAX_READ_SIZE)
            {
              m_dataBuffer.clear();
              break;
            }
          }
        }
      }
    }
  }
}

bool CGameInfoLoader::GetMemoryStruct(retro_game_info& info) const
{
  if (!m_dataBuffer.empty())
  {
    info.path = NULL;
    info.data = m_dataBuffer.data();
    info.size = m_dataBuffer.size();
    info.meta = NULL;
    return true;
  }
  return false;
}

bool CGameInfoLoader::GetPathStruct(retro_game_info& info) const
{
  info.path = m_pathBuffer.c_str();
  info.data = NULL;
  info.size = 0;
  info.meta = NULL;
  return true;
}
