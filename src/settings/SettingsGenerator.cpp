/*
 *  Copyright (C) 2016-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SettingsGenerator.h"

#include <fstream>

using namespace LIBRETRO;

CSettingsGenerator::CSettingsGenerator(const std::string& generatedDir)
{
  m_strFilePath = generatedDir + "/" SETTINGS_GENERATED_SETTINGS_NAME;
}

bool CSettingsGenerator::GenerateSettings(const LibretroSettings& settings)
{
  std::ofstream file(m_strFilePath, std::ios::trunc);
  if (!file.is_open())
    return false;

  unsigned int settingId = SETTING_ID_START;

  file << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>" << std::endl;
  file << "<settings>" << std::endl;
  file << "\t<category label=\"" << settingId++ << "\">" << std::endl;

  for (const auto& setting : settings)
  {
    const std::string& key = setting.first;
    const std::string& strValues = setting.second.ValuesStr();
    const std::string& defaultValue = setting.second.DefaultValue();

    file << "\t\t<setting label=\"" << settingId++ << "\" type=\"select\" id=\"" << key << "\" values=\"" << strValues << "\" default=\"" << defaultValue << "\"/>" << std::endl;
  }

  file << "\t</category>" << std::endl;
  file << "</settings>" << std::endl;

  file.close();

  return true;
}
