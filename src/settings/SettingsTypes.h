/*
 *  Copyright (C) 2016-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "LibretroSetting.h"

#include <map>
#include <string>

/*!
 * \brief Directory name for generated settings and language files
 */
#define SETTINGS_GENERATED_DIRECTORY_NAME  "generated"

/*!
 * \brief File name of the generated settings.xml file
 */
#define SETTINGS_GENERATED_SETTINGS_NAME  "settings.xml"

/*!
 * \brief File name and subdirectory of the generated language file
 */
#define SETTINGS_GENERATED_LANGUAGE_SUBDIR          "language"
#define SETTINGS_GENERATED_LANGUAGE_ENGLISH_SUBDIR  "English"
#define SETTINGS_GENERATED_LANGUAGE_NAME            "strings.po"

#define SETTING_ID_START  30000

namespace LIBRETRO
{
  typedef std::string SettingKey;
  typedef std::map<SettingKey, CLibretroSetting> LibretroSettings;
}
