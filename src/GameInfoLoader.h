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
#pragma once

#include "libretro/libretro.h"

#include <stdint.h>
#include <string>
#include <vector>

namespace ADDON { class CHelper_libXBMC_addon; }

namespace LIBRETRO
{
  /*!
   * Libretro's retro_load_game() and retro_load_game_special() accept a
   * parameter of type retro_game_info. By setting various fields to NULL,
   * the libretro core can be instructed to load via memory or via path. This
   * class abstracts this logic by accepting a path and generating the
   * appropriate retro_game_info structs.
   */
  class CGameInfoLoader
  {
  public:
    CGameInfoLoader(const char* path, ADDON::CHelper_libXBMC_addon* XBMC, bool bSupportsVFS);

    bool Load(void);

    /*!
     * Get the struct that instructs libretro to load via memory. Returns false
     * if loading via memory is not a valid strategy for this libretro core
     * (doesn't support VFS, file is too big / doesn't exist, etc).
     */
    bool GetMemoryStruct(retro_game_info& info) const;

    /*!
     * As a fallback, this gets a struct that instructs libretro to load via
     * path. This always returns true.
     */
    bool GetPathStruct(retro_game_info& info) const;

  private:
    const std::string                   m_path;
    ADDON::CHelper_libXBMC_addon* const m_xbmc;
    const bool                          m_bSupportsVfs;
    std::vector<uint8_t>                m_dataBuffer;
  };
} // namespace LIBRETRO
