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
  class CSettingsGenerator
  {
  public:
    CSettingsGenerator(const std::string& generatedDir);

    bool GenerateSettings(const LibretroSettings& settings);

  private:
    std::string m_strFilePath;
  };
}
