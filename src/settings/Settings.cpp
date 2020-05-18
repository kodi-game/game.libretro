/*
 *  Copyright (C) 2016-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Settings.h"

using namespace LIBRETRO;

#define SETTING_CROP_OVERSCAN  "cropoverscan"

CSettings::CSettings(void)
  : m_bInitialized(false),
    m_bCropOverscan(false)
{
}

CSettings& CSettings::Get(void)
{
  static CSettings _instance;
  return _instance;
}

void CSettings::SetSetting(const std::string& strName, const kodi::CSettingValue& value)
{
  if (strName == SETTING_CROP_OVERSCAN)
  {
    m_bCropOverscan = value.GetBoolean();
    //dsyslog("Setting \"%s\" set to %f", SETTING_CROP_OVERSCAN, m_bCropOverscan ? "true" : "false");
  }

  m_bInitialized = true;
}
