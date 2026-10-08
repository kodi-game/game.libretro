/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Settings.h"
#include "SettingsXML.h"
#include "filesystem/FilesystemUtils.h"

#include <kodi/Filesystem.h>

using namespace LIBRETRO;

#define SETTING_CROP_OVERSCAN  "cropoverscan"

namespace
{
constexpr auto SETTING_SHARE_SYSTEM = "sharesystemdirectory";
constexpr auto SETTING_SHARED_SYSTEM_PATH = "sharedsystemdirectory";
constexpr auto DEFAULT_SHARED_SYSTEM_PATH =
    "special://profile/addon_data/game.libretro/resources/system";
} // namespace

CSettings::CSettings(void)
  : m_bCropOverscan(false)
{
}

CSettings& CSettings::Get(void)
{
  static CSettings _instance;
  return _instance;
}

void CSettings::ReadAddonSettings()
{
  CSettingsXML settings;
  settings.Load();

  m_bCropOverscan = settings.GetBool(SETTING_CROP_OVERSCAN, m_bCropOverscan);

  m_sharedSystemDirectory.clear();
  if (!settings.GetBool(SETTING_SHARE_SYSTEM, false))
    return;

  m_sharedSystemDirectory = CFilesystemUtils::CoreDirectory(kodi::vfs::TranslateSpecialProtocol(
      settings.GetString(SETTING_SHARED_SYSTEM_PATH, DEFAULT_SHARED_SYSTEM_PATH)));
}
