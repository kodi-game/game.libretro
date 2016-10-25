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

void CSettings::SetSetting(const std::string& strName, const void* value)
{
  if (strName == SETTING_CROP_OVERSCAN)
  {
    m_bCropOverscan = *static_cast<const bool*>(value);
    //dsyslog("Setting \"%s\" set to %f", SETTING_CROP_OVERSCAN, m_bCropOverscan ? "true" : "false");
  }

  m_bInitialized = true;
}
