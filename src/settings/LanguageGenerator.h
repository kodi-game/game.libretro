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
  class CLanguageGenerator
  {
  public:
    CLanguageGenerator(const std::string& addonId, const std::string& generatedDir);

    /*!
     * \brief Write strings.po
     *
     * \param strings The text of each string ID, counting up from SETTING_ID_START
     */
    bool GenerateLanguage(const std::vector<std::string>& strings);

  private:
    std::string m_strAddonId;
    std::string m_strFilePath;
  };
}
