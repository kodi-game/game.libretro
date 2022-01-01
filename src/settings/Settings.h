/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/AddonBase.h>
#include <string>

namespace LIBRETRO
{
  class CSettings
  {
  private:
    CSettings(void);

  public:
    static CSettings& Get(void);

    bool IsInitialized(void) const { return m_bInitialized; }

    void SetSetting(const std::string& strName, const kodi::addon::CSettingValue& value);

    /*!
     * \brief True if the libretro core should crop overscan
     */
    bool CropOverscan(void) const { return m_bCropOverscan; }

  private:
    bool  m_bInitialized;
    bool  m_bCropOverscan;
  };
}
