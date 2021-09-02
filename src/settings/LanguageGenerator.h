/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "SettingsTypes.h"

#include <string>

namespace LIBRETRO
{
  class CLanguageGenerator
  {
  public:
    CLanguageGenerator(const std::string& addonId, const std::string& generatedDir);

    bool GenerateLanguage(const LibretroSettings& settings);

  private:
    std::string m_strAddonId;
    std::string m_strFilePath;
  };
}
