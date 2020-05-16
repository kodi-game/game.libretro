/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro/libretro.h"

#include <stdint.h>
#include <string>
#include <vector>

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
    CGameInfoLoader(const std::string& path, bool bSupportsVFS);

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
    const bool                          m_bSupportsVfs;
    std::vector<uint8_t>                m_dataBuffer;
  };
} // namespace LIBRETRO
