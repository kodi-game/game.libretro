/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroSettings.h"
#include "libretro-common/libretro.h"
#include "log/Log.h"
#include "client.h"

#include <algorithm>
#include <utility>

using namespace LIBRETRO;

CLibretroSettings::CLibretroSettings() :
  m_bChanged(true)
{
}

void CLibretroSettings::Deinitialize()
{
  std::unique_lock<std::mutex> lock(m_mutex);

  m_settings.clear();
  m_bChanged = true;
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
        }
      }
      else
      {
        esyslog("Setting %s not found by Kodi", setting.Key().c_str());
      }

      m_settings.insert(std::make_pair(setting.Key(), std::move(setting)));
    }

    m_bChanged = true;
  }
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
} // namespace

void CLibretroSettings::AddSetting(CLibretroSetting setting)
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
    }
  }
  else
  {
    esyslog("Setting %s not found by Kodi", setting.Key().c_str());
  }

  m_settings.insert(std::make_pair(setting.Key(), std::move(setting)));
}

void CLibretroSettings::SetAllSettings(const retro_core_option_definition* definitions)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_settings.empty())
  {
    for (const retro_core_option_definition* definition = definitions;
         definition != nullptr && definition->key != nullptr; definition++)
    {
      AddSetting(CLibretroSetting(definition->key, definition->desc,
                                  GetOptionValues(definition->values), definition->default_value));
    }

    m_bChanged = true;
  }
}

void CLibretroSettings::SetAllSettings(const retro_core_option_v2_definition* definitions)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_settings.empty())
  {
    for (const retro_core_option_v2_definition* definition = definitions;
         definition != nullptr && definition->key != nullptr; definition++)
    {
      AddSetting(CLibretroSetting(definition->key, definition->desc,
                                  GetOptionValues(definition->values), definition->default_value));
    }

    m_bChanged = true;
  }
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

  // Check to make sure value is a valid value reported by libretro
  auto it = m_settings.find(name);
  if (it == m_settings.end())
  {
    esyslog("Kodi setting %s unknown to libretro!", name.c_str());
  }
  else if (it->second.CurrentValue() != value)
  {
    it->second.SetCurrentValue(value);
    m_bChanged = true;
  }
}
