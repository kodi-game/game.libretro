/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "LibretroSetting.h"

#include <map>
#include <string>

namespace LIBRETRO
{
  typedef std::string SettingKey;
  typedef std::map<SettingKey, CLibretroSetting> LibretroSettings;
}
