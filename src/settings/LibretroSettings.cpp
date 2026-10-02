/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroSettings.h"
#include "LanguageGenerator.h"
#include "SettingsGenerator.h"
#include "libretro-common/libretro.h"
#include "log/Log.h"
#include "client.h"

#include <kodi/Filesystem.h>

#include <algorithm>
#include <assert.h>
#include <utility>

using namespace LIBRETRO;

CLibretroSettings::CLibretroSettings() :
  m_addon(nullptr),
  m_bChanged(true),
  m_bGenerated(false)
{
}

void CLibretroSettings::Initialize(CGameLibRetro* addon)
{
  m_addon = addon;
  assert(m_addon != nullptr);

  m_profileDirectory = m_addon->ProfileDirectory();
}

void CLibretroSettings::Deinitialize()
{
  std::unique_lock<std::mutex> lock(m_mutex);

  m_addon = nullptr;
  m_profileDirectory.clear();
  m_settings.clear();
  m_order.clear();
  m_categories.clear();
  m_bChanged = true;
  m_bGenerated = false;
}

bool CLibretroSettings::Changed()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  return m_bChanged;
}

void CLibretroSettings::SetUnchanged()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_bChanged = false;
}

void CLibretroSettings::SetAllSettings(const retro_variable* libretroVariables)
{
  // Keep track of whether Kodi has the correct settings
  bool bValid = true;

  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_settings.empty())
  {
    for (const retro_variable* variable = libretroVariables; variable && variable->key && variable->value; variable++)
    {
      CLibretroSetting setting(variable);

      if (setting.Values().empty())
      {
        esyslog("Setting \"%s\": No pipe-delimited options: \"%s\"", variable->key, variable->value);
        continue;
      }

      // Query current value for setting from the frontend
      std::string valueBuf;
      if (kodi::addon::CheckSettingString(variable->key, valueBuf))
      {
        if (std::find(setting.Values().begin(), setting.Values().end(), valueBuf) != setting.Values().end())
        {
          dsyslog("Setting %s has value \"%s\" in Kodi",  setting.Key().c_str(), valueBuf.c_str());
          setting.SetCurrentValue(valueBuf);
        }
        else
        {
          esyslog("Setting %s: invalid value \"%s\" (values are: %s)", setting.Key().c_str(), valueBuf.c_str(), variable->value);
          bValid = false;
        }
      }
      else
      {
        esyslog("Setting %s not found by Kodi", setting.Key().c_str());
        bValid = false;
      }

      const SettingKey key = setting.Key();
      if (m_settings.insert(std::make_pair(key, std::move(setting))).second)
        m_order.push_back(key);
    }

    m_bChanged = true;
  }

  if (!bValid)
    GenerateSettings();
}

namespace
{
/*!
 * \brief Collect the values a core options entry offers
 *
 * The array is terminated by an entry with a null value, and the label is for
 * display only -- what a core reads back is the value.
 */
std::vector<std::string> GetOptionValues(const retro_core_option_value* values)
{
  std::vector<std::string> result;

  for (const retro_core_option_value* value = values; value != nullptr && value->value != nullptr;
       value++)
    result.emplace_back(value->value);

  return result;
}

/*!
 * \brief Collect the display text of each value, empty where it has none
 */
std::vector<std::string> GetOptionLabels(const retro_core_option_value* values)
{
  std::vector<std::string> result;

  for (const retro_core_option_value* value = values; value != nullptr && value->value != nullptr;
       value++)
    result.emplace_back(value->label != nullptr ? value->label : "");

  return result;
}
} // namespace

void CLibretroSettings::AddSetting(CLibretroSetting setting, bool& bValid)
{
  if (setting.Values().empty())
  {
    esyslog("Setting \"%s\": no values", setting.Key().c_str());
    return;
  }

  std::string valueBuf;
  if (kodi::addon::CheckSettingString(setting.Key(), valueBuf))
  {
    if (std::find(setting.Values().begin(), setting.Values().end(), valueBuf) !=
        setting.Values().end())
    {
      dsyslog("Setting %s has value \"%s\" in Kodi", setting.Key().c_str(), valueBuf.c_str());
      setting.SetCurrentValue(valueBuf);
    }
    else
    {
      esyslog("Setting %s: invalid value \"%s\" (values are: %s)", setting.Key().c_str(),
              valueBuf.c_str(), setting.ValuesStr().c_str());
      bValid = false;
    }
  }
  else
  {
    esyslog("Setting %s not found by Kodi", setting.Key().c_str());
    bValid = false;
  }

  const SettingKey key = setting.Key();
  if (m_settings.insert(std::make_pair(key, std::move(setting))).second)
    m_order.push_back(key);
}

void CLibretroSettings::SetAllSettings(const retro_core_option_definition* definitions)
{
  bool bValid = true;

  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_settings.empty())
  {
    for (const retro_core_option_definition* definition = definitions;
         definition != nullptr && definition->key != nullptr; definition++)
    {
      AddSetting(CLibretroSetting(definition->key, definition->desc,
                                  GetOptionValues(definition->values), definition->default_value,
                                  definition->info, nullptr, GetOptionLabels(definition->values)),
                 bValid);
    }

    m_bChanged = true;
  }

  if (!bValid)
    GenerateSettings();
}

void CLibretroSettings::SetAllSettings(const retro_core_option_v2_definition* definitions,
                                       const retro_core_option_v2_category* categories)
{
  bool bValid = true;

  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_settings.empty())
  {
    for (const retro_core_option_v2_category* category = categories;
         category != nullptr && category->key != nullptr; category++)
    {
      m_categories.push_back({category->key, category->desc != nullptr ? category->desc : "",
                              category->info != nullptr ? category->info : ""});
    }

    for (const retro_core_option_v2_definition* definition = definitions;
         definition != nullptr && definition->key != nullptr; definition++)
    {
      AddSetting(CLibretroSetting(definition->key, definition->desc,
                                  GetOptionValues(definition->values), definition->default_value,
                                  definition->info, definition->category_key,
                                  GetOptionLabels(definition->values)),
                 bValid);
    }

    m_bChanged = true;
  }

  if (!bValid)
    GenerateSettings();
}

const char* CLibretroSettings::GetCurrentValue(const std::string& settingName)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  auto it = m_settings.find(settingName);
  if (it == m_settings.end())
  {
    esyslog("Unknown setting ID: %s", settingName.c_str());
    return "";
  }

  return it->second.CurrentValue().c_str();
}

void CLibretroSettings::SetCurrentValue(const std::string& name, const std::string& value)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_settings.empty())
  {
    // RETRO_ENVIRONMENT_SET_VARIABLES hasn't been called yet. We don't need to
    // record the setting now because it will be retrieved from the frontend
    // later.
    return;
  }

  // Keep track of whether Kodi has the correct settings
  bool bValid = true;

  // Check to make sure value is a valid value reported by libretro
  auto it = m_settings.find(name);
  if (it == m_settings.end())
  {
    esyslog("Kodi setting %s unknown to libretro!", name.c_str());
    bValid = false;
  }
  else if (it->second.CurrentValue() != value)
  {
    it->second.SetCurrentValue(value);
    m_bChanged = true;
  }

  if (!bValid)
    GenerateSettings();
}

void CLibretroSettings::GenerateSettings()
{
  if (!m_bGenerated && !m_settings.empty())
  {
    isyslog("Invalid settings detected, generating new settings and language files");

    std::string generatedPath = m_profileDirectory;

    std::string addonId = kodi::vfs::GetFileName(generatedPath);

    generatedPath += "/" SETTINGS_GENERATED_DIRECTORY_NAME;

    // Ensure folder exists
    if (!kodi::vfs::DirectoryExists(generatedPath))
    {
      dsyslog("Creating directory for settings and language files: %s", generatedPath.c_str());
      kodi::vfs::CreateDirectory(generatedPath);
    }

    bool bSuccess = false;

    std::vector<const CLibretroSetting*> settings;
    for (const SettingKey& key : m_order)
      settings.push_back(&m_settings.at(key));

    std::vector<std::string> strings;
    CSettingsGenerator settingsGen(generatedPath);
    if (!settingsGen.GenerateSettings(addonId, m_categories, settings, strings))
      esyslog("Failed to generate %s", SETTINGS_GENERATED_SETTINGS_NAME);
    else
      bSuccess = true;

    generatedPath += "/" SETTINGS_GENERATED_LANGUAGE_SUBDIR;

    // Ensure language folder exists
    if (!kodi::vfs::DirectoryExists(generatedPath))
    {
      dsyslog("Creating directory for settings and language files: %s", generatedPath.c_str());
      kodi::vfs::CreateDirectory(generatedPath);
    }

    generatedPath += "/" SETTINGS_GENERATED_LANGUAGE_ENGLISH_SUBDIR;

    // Ensure English folder exists
    if (!kodi::vfs::DirectoryExists(generatedPath))
    {
      dsyslog("Creating directory for settings and language files: %s", generatedPath.c_str());
      kodi::vfs::CreateDirectory(generatedPath);
    }

    CLanguageGenerator languageGen(addonId, generatedPath);
    if (!languageGen.GenerateLanguage(strings))
      esyslog("Failed to generate %s", SETTINGS_GENERATED_LANGUAGE_NAME);
    else
      bSuccess = true;

    if (bSuccess)
      isyslog("Settings and language files have been placed in %s", generatedPath.c_str());

    m_bGenerated = true;
  }
}
