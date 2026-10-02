/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LanguageGenerator.h"

#include <fstream>

using namespace LIBRETRO;

namespace
{
// Help text runs to several lines and quotes things, and a raw newline or
// quote would end the msgid early
std::string EscapePo(const std::string& text)
{
  std::string result;

  for (char c : text)
  {
    switch (c)
    {
      case '\\': result += "\\\\"; break;
      case '"': result += "\\\""; break;
      case '\t': result += "\\t"; break;
      case '\n': result += "\\n"; break;
      case '\r': break;
      default: result += c; break;
    }
  }

  return result;
}
} // namespace

CLanguageGenerator::CLanguageGenerator(const std::string& addonId, const std::string& generatedDir) :
  m_strAddonId(addonId)
{
  m_strFilePath = generatedDir + "/" SETTINGS_GENERATED_LANGUAGE_NAME;
}

bool CLanguageGenerator::GenerateLanguage(const std::vector<std::string>& strings)
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
  file << "\"Report-Msgid-Bugs-To: translations@kodi.tv\\n\"" << std::endl;
  file << "\"POT-Creation-Date: 2016-10-25 17:00+8\\n\"" << std::endl;
  file << "\"PO-Revision-Date: 2016-10-25 17:00+8\\n\"" << std::endl;
  file << "\"Last-Translator: Kodi Translation Team\\n\"" << std::endl;
  file << "\"Language-Team: English (United Kingdom) (https://kodi.weblate.cloud/languages/en_gb/)\\n\"" << std::endl;
  file << "\"MIME-Version: 1.0\\n\"" << std::endl;
  file << "\"Content-Type: text/plain; charset=UTF-8\\n\"" << std::endl;
  file << "\"Content-Transfer-Encoding: 8bit\\n\"" << std::endl;
  file << "\"Language: en\\n\"" << std::endl;
  file << "\"Plural-Forms: nplurals=2; plural=(n != 1);\\n\"" << std::endl;
  file << std::endl;

  unsigned int settingId = SETTING_ID_START;

  for (const std::string& text : strings)
  {
    file << "msgctxt \"#" << settingId++ << "\"" << std::endl;
    file << "msgid \"" << EscapePo(text) << "\"" << std::endl;
    file << "msgstr \"\"" << std::endl;
    file << std::endl;
  }

  file.close();

  return true;
}
