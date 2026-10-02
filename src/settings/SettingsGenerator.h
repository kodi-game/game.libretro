/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "SettingsTypes.h"

#include <string>
#include <vector>

namespace LIBRETRO
{
  class CSettingsGenerator
  {
  public:
    CSettingsGenerator(const std::string& generatedDir);

    /*!
     * \brief Write settings.xml in the form the add-ons ship
     *
     * \param settings The settings in the order the core declared them
     * \param strings Receives the text of each string ID the file refers to,
     *                counting up from SETTING_ID_START
     */
    bool GenerateSettings(const std::string& addonId,
                          const std::vector<LibretroSettingCategory>& categories,
                          const std::vector<const CLibretroSetting*>& settings,
                          std::vector<std::string>& strings);

  private:
    std::string m_strFilePath;
  };
}
