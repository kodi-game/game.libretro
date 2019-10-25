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

#include "LibretroSettings.h"
#include "LanguageGenerator.h"
#include "SettingsGenerator.h"
#include "libretro/libretro.h"
#include "log/Log.h"
#include "client.h"

#include <kodi/Filesystem.h>

#include <algorithm>
#include <assert.h>
#include <utility>

using namespace LIBRETRO;
using namespace P8PLATFORM;

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
  m_addon = nullptr;
}

bool CLibretroSettings::Changed()
{
  CLockObject lock(m_mutex);
  return m_bChanged;
}

void CLibretroSettings::SetUnchanged()
{
  CLockObject lock(m_mutex);
  m_bChanged = false;
}

void CLibretroSettings::SetAllSettings(const retro_variable* libretroVariables)
{
  // Keep track of whether Kodi has the correct settings
  bool bValid = true;

  CLockObject lock(m_mutex);

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
      if (kodi::CheckSettingString(variable->key, valueBuf))
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

      m_settings.insert(std::make_pair(setting.Key(), std::move(setting)));
    }

    m_bChanged = true;
  }

  if (!bValid)
    GenerateSettings();
}

const char* CLibretroSettings::GetCurrentValue(const std::string& settingName)
{
  CLockObject lock(m_mutex);

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
  CLockObject lock(m_mutex);

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

    CSettingsGenerator settingsGen(generatedPath);
    if (!settingsGen.GenerateSettings(m_settings))
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
    if (!languageGen.GenerateLanguage(m_settings))
      esyslog("Failed to generate %s", SETTINGS_GENERATED_LANGUAGE_NAME);
    else
      bSuccess = true;

    if (bSuccess)
      isyslog("Settings and language files have been placed in %s", generatedPath.c_str());

    m_bGenerated = true;
  }
}
