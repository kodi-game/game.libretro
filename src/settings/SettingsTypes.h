/*
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
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
