/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SettingsGenerator.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <utility>

using namespace LIBRETRO;

namespace
{
std::string EscapeXml(const std::string& text)
{
  std::string result;

  for (char c : text)
  {
    switch (c)
    {
      case '&': result += "&amp;"; break;
      case '<': result += "&lt;"; break;
      case '>': result += "&gt;"; break;
      case '"': result += "&quot;"; break;
      case '\'': result += "&apos;"; break;
      default: result += c; break;
    }
  }

  return result;
}
} // namespace

CSettingsGenerator::CSettingsGenerator(const std::string& generatedDir)
{
  m_strFilePath = generatedDir + "/" SETTINGS_GENERATED_SETTINGS_NAME;
}

bool CSettingsGenerator::GenerateSettings(const std::string& addonId,
                                          const std::vector<LibretroSettingCategory>& categories,
                                          const std::vector<const CLibretroSetting*>& settings,
                                          std::vector<std::string>& strings)
{
  std::ofstream file(m_strFilePath, std::ios::trunc);
  if (!file.is_open())
    return false;

  std::map<std::string, unsigned int> stringIds;
  auto StringId = [&strings, &stringIds](const std::string& text)
  {
    auto it = stringIds.find(text);
    if (it == stringIds.end())
    {
      it = stringIds.emplace(text, SETTING_ID_START + strings.size()).first;
      strings.push_back(text);
    }
    return it->second;
  };

  // An option outside the categories the core declares goes in Kodi's own
  // "General", first, as the add-ons' settings have it
  std::vector<std::pair<const LibretroSettingCategory*, std::vector<const CLibretroSetting*>>>
      groups(1);
  std::map<std::string, size_t> groupIndex;
  for (const LibretroSettingCategory& category : categories)
  {
    if (groupIndex.emplace(category.key, groups.size()).second)
      groups.emplace_back(&category, std::vector<const CLibretroSetting*>{});
  }

  for (const CLibretroSetting* setting : settings)
  {
    auto it = groupIndex.find(setting->Category());
    groups[it != groupIndex.end() ? it->second : 0].second.push_back(setting);
  }

  file << "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>" << std::endl;
  file << "<settings version=\"1\">" << std::endl;
  file << "\t<section id=\"" << EscapeXml(addonId) << "\">" << std::endl;

  for (const auto& group : groups)
  {
    if (group.second.empty())
      continue;

    const LibretroSettingCategory* category = group.first;
    if (category == nullptr)
      file << "\t\t<category id=\"general\" label=\"128\">" << std::endl;
    else
    {
      file << "\t\t<category id=\"" << EscapeXml(category->key) << "\" label=\""
           << StringId(category->description.empty() ? category->key : category->description)
           << "\"";
      if (!category->info.empty())
        file << " help=\"" << StringId(category->info) << "\"";
      file << ">" << std::endl;
    }
    file << "\t\t\t<group id=\"1\">" << std::endl;

    for (const CLibretroSetting* setting : group.second)
    {
      file << "\t\t\t\t<setting id=\"" << EscapeXml(setting->Key()) << "\" type=\"string\" label=\""
           << StringId(setting->Description().empty() ? setting->Key() : setting->Description())
           << "\"";
      if (!setting->Info().empty())
        file << " help=\"" << StringId(setting->Info()) << "\"";
      file << ">" << std::endl;
      file << "\t\t\t\t\t<level>0</level>" << std::endl;
      file << "\t\t\t\t\t<default>" << EscapeXml(setting->DefaultValue()) << "</default>"
           << std::endl;
      file << "\t\t\t\t\t<constraints>" << std::endl;
      file << "\t\t\t\t\t\t<options>" << std::endl;

      // Kodi hides the unlabelled entries of a list that labels any, so it is
      // all or nothing
      const std::vector<std::string>& values = setting->Values();
      const std::vector<std::string>& labels = setting->Labels();
      const bool labelled = std::any_of(labels.begin(), labels.end(),
                                        [](const std::string& label) { return !label.empty(); });

      for (size_t i = 0; i < values.size(); i++)
      {
        file << "\t\t\t\t\t\t\t<option";
        if (labelled)
        {
          const bool hasLabel = i < labels.size() && !labels[i].empty();
          file << " label=\"" << StringId(hasLabel ? labels[i] : values[i]) << "\"";
        }
        file << ">" << EscapeXml(values[i]) << "</option>" << std::endl;
      }

      file << "\t\t\t\t\t\t</options>" << std::endl;
      file << "\t\t\t\t\t</constraints>" << std::endl;
      file << "\t\t\t\t\t<control type=\"list\" format=\"string\" />" << std::endl;
      file << "\t\t\t\t</setting>" << std::endl;
    }

    file << "\t\t\t</group>" << std::endl;
    file << "\t\t</category>" << std::endl;
  }

  file << "\t</section>" << std::endl;
  file << "</settings>" << std::endl;

  file.close();

  return true;
}
