/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>

namespace LIBRETRO
{
  class CSettings
  {
  private:
    CSettings(void);

  public:
    static CSettings& Get(void);

    /*!
     * \brief Read the settings that belong to game.libretro rather than the core
     */
    void ReadAddonSettings();

    /*!
     * \brief True if the libretro core should crop overscan
     */
    bool CropOverscan(void) const { return m_bCropOverscan; }

    /*!
     * \brief The system folder shared by every emulator, or empty if sharing is off
     */
    const std::string& SharedSystemDirectory() const { return m_sharedSystemDirectory; }

  private:
    bool  m_bCropOverscan;
    std::string m_sharedSystemDirectory;
  };
}
