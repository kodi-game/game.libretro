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

#include "LanguageGenerator.h"

#include <fstream>

using namespace LIBRETRO;

CLanguageGenerator::CLanguageGenerator(const std::string& addonId, const std::string& generatedDir) :
  m_strAddonId(addonId)
{
  m_strFilePath = generatedDir + "/" SETTINGS_GENERATED_LANGUAGE_NAME;
}

bool CLanguageGenerator::GenerateLanguage(const LibretroSettings& settings)
{
  if (m_strAddonId.empty())
    return false;

  std::ofstream file(m_strFilePath, std::ios::trunc);
  if (!file.is_open())
    return false;

  file << "# " << m_strAddonId << " language file" << std::endl;
  file << "# Addon Name: " << m_strAddonId << std::endl;
  file << "# Addon id: " << m_strAddonId << std::endl;
  file << "# Addon Provider: libretro" << std::endl;
  file << "msgid \"\"" << std::endl;
  file << "msgstr \"\"" << std::endl;
  file << "\"Project-Id-Version: " << m_strAddonId << "\\n\"" << std::endl;
  file << "\"Report-Msgid-Bugs-To: alanwww1@xbmc.org\\n\"" << std::endl;
  file << "\"POT-Creation-Date: 2016-10-25 17:00+8\\n\"" << std::endl;
  file << "\"PO-Revision-Date: 2016-10-25 17:00+8\\n\"" << std::endl;
  file << "\"Last-Translator: Kodi Translation Team\\n\"" << std::endl;
  file << "\"Language-Team: English (http://www.transifex.com/projects/p/xbmc-addons/language/en/)\\n\"" << std::endl;
  file << "\"MIME-Version: 1.0\\n\"" << std::endl;
  file << "\"Content-Type: text/plain; charset=UTF-8\\n\"" << std::endl;
  file << "\"Content-Transfer-Encoding: 8bit\\n\"" << std::endl;
  file << "\"Language: en\\n\"" << std::endl;
  file << "\"Plural-Forms: nplurals=2; plural=(n != 1);\\n\"" << std::endl;
  file << std::endl;

  unsigned int settingId = SETTING_ID_START;

  // Category name
  file << "msgctxt \"#" << settingId++ << "\"" << std::endl;
  file << "msgid \"Settings\"" << std::endl;
  file << "msgstr \"\"" << std::endl;
  file << std::endl;

  for (const auto& setting : settings)
  {
    // TODO: xml-encode description
    const std::string& description = setting.second.Description();

    file << "msgctxt \"#" << settingId++ << "\"" << std::endl;
    file << "msgid \"" << description << "\"" << std::endl;
    file << "msgstr \"\"" << std::endl;
    file << std::endl;
  }

  file.close();

  return true;
}
