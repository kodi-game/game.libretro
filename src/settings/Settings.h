/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

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

  private:
    bool  m_bCropOverscan;
  };
}
