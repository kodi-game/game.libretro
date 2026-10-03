/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Settings.h"
#include "SettingsXML.h"

using namespace LIBRETRO;

#define SETTING_CROP_OVERSCAN  "cropoverscan"

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
}
