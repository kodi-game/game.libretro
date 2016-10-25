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
