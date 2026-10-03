/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SettingsXML.h"

#include <kodi/Filesystem.h>
#include <tinyxml.h>

using namespace LIBRETRO;

namespace
{
constexpr auto SETTINGS_FILE = "special://profile/addon_data/game.libretro/settings.xml";
} // namespace

void CSettingsXML::Load()
{
  m_values.clear();

  TiXmlDocument settingsXml;
  if (!settingsXml.LoadFile(kodi::vfs::TranslateSpecialProtocol(SETTINGS_FILE)) ||
      settingsXml.RootElement() == nullptr)
    return;

  for (const TiXmlElement* setting = settingsXml.RootElement()->FirstChildElement("setting");
       setting != nullptr; setting = setting->NextSiblingElement("setting"))
  {
    const char* id = setting->Attribute("id");
    const char* value = setting->GetText();
    if (id != nullptr && value != nullptr)
      m_values[id] = value;
  }
}

bool CSettingsXML::GetBool(const std::string& id, bool defaultValue) const
{
  const auto it = m_values.find(id);
  return it != m_values.end() ? it->second == "true" : defaultValue;
}

std::string CSettingsXML::GetString(const std::string& id, const std::string& defaultValue) const
{
  const auto it = m_values.find(id);
  return it != m_values.end() ? it->second : defaultValue;
}
