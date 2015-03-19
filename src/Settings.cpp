/*
 *      Copyright (C) 2015 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "Settings.h"
#include "LibretroEnvironment.h"

#include "kodi/libXBMC_addon.h"

#include <algorithm>

using namespace ADDON;
using namespace LIBRETRO;
using namespace PLATFORM;

CSettings::CSettings(void)
  : m_bSettingsChanged(false)
{
}

CSettings& CSettings::Get(void)
{
  static CSettings _instance;
  return _instance;
}

bool CSettings::HasAddonSetting(const std::string& strKey) const
{
  CLockObject lock(m_settingsMutex);

  return m_addonSettings.find(strKey) != m_addonSettings.end();
}

const char* CSettings::GetAddonSetting(const std::string& strKey) const
{
  CLockObject lock(m_settingsMutex);

  std::map<std::string, std::string>::const_iterator it = m_addonSettings.find(strKey);
  if (it != m_addonSettings.end())
    return it->second.c_str();

  return "";
}

void CSettings::SetAddonSetting(const std::string& name, const std::string& value)
{
  CLockObject lock(m_settingsMutex);

  if (m_options.empty())
  {
    // RETRO_ENVIRONMENT_SET_VARIABLES hasn't been called yet. We don't need to
    // record the setting now, because m_addonSettings will be initialized
    // alongside m_options.
    return;
  }

  // Check to make sure value is a valid value reported by libretro
  if (m_options.find(name) == m_options.end())
  {
    if (CLibretroEnvironment::Get().GetXBMC())
      CLibretroEnvironment::Get().GetXBMC()->Log(LOG_ERROR, "XBMC setting \"%s\" unknown to libretro!",
                                                 name.c_str());
    return;
  }

  std::vector<std::string>& values = m_options[name];
  if (std::find(values.begin(), values.end(), value) == values.end())
  {
    if (CLibretroEnvironment::Get().GetXBMC())
      CLibretroEnvironment::Get().GetXBMC()->Log(LOG_ERROR, "\"%s\" is not a valid value for setting \"%s\"",
                                                 value.c_str(), name.c_str());
    return;
  }

  if (m_addonSettings[name] != value)
  {
    m_addonSettings[name] = value;
    m_bSettingsChanged = true;
  }
}

bool CSettings::HasOption(const std::string& setting, const std::string& option) const
{
  CLockObject lock(m_settingsMutex);

  std::map<std::string, std::vector<std::string> >::const_iterator it = m_options.find(setting);
  if (it != m_options.end())
  {
    const std::vector<std::string>& options = it->second;
    if (std::find(options.begin(), options.end(), option) != options.end())
      return true;
  }
  return false;
}

void CSettings::SetOptions(const std::string& setting, const std::vector<std::string>& options)
{
  CLockObject lock(m_settingsMutex);

  m_options[setting] = options;

  FetchAddonSettings(setting, options[0]); // Default to first value per libretro api

  m_bSettingsChanged = true;
}

void CSettings::FetchAddonSettings(const std::string& setting, const std::string& strDefault)
{
  if (!CLibretroEnvironment::Get().GetXBMC())
    return;

  // Query value for addon in XBMC
  char valueBuf[1024] = { };
  if (CLibretroEnvironment::Get().GetXBMC()->GetSetting(setting.c_str(), valueBuf))
  {
    if (HasOption(setting, valueBuf))
    {
      CLibretroEnvironment::Get().GetXBMC()->Log(LOG_DEBUG, "Setting \"%s\" for this add-on has value \"%s\"",
                                                 setting.c_str(), valueBuf);
      m_addonSettings[setting] = valueBuf;
    }
    else
    {
      CLibretroEnvironment::Get().GetXBMC()->Log(LOG_ERROR, "Setting \"%s\": invalid value \"%s\"",
                                                 setting.c_str(), valueBuf);
      m_addonSettings[setting] = strDefault;
    }
  }
  else
  {
    CLibretroEnvironment::Get().GetXBMC()->Log(LOG_ERROR, "Setting %s not found by XBMC",
                                               setting.c_str());
    m_addonSettings[setting] = strDefault;
  }
}

bool CSettings::IsChanged(void) const
{
  CLockObject lock(m_settingsMutex);

  return m_bSettingsChanged;
}

void CSettings::SetChanged(bool bChanged)
{
  CLockObject lock(m_settingsMutex);

  m_bSettingsChanged = bChanged;
}

std::vector<std::string> CSettings::ParseOptions(std::string strLibretroOptions)
{
  std::vector<std::string> vecOptions;

  // Example retro_variable:
  //      { "foo_option", "Speed hack coprocessor X; false|true" }

  // Text before first ';' is description. This ';' must be followed by
  // a space, and followed by a list of possible values split up with '|'.
  // Here, we break up this horrible messy string into the m_variables
  // data structure and initialize m_settings with XBMC's settings.

  // First look for ; separating the description from the pipe-separated values
  size_t pos;
  if ((pos = strLibretroOptions.find(';')) != std::string::npos)
  {
    pos++;
    while (pos < strLibretroOptions.size() && strLibretroOptions[pos] == ' ')
      pos++;
    strLibretroOptions = strLibretroOptions.substr(pos);
  }

  // Split the values on | delimiter and build m_variables array
  while (!strLibretroOptions.empty())
  {
    std::string strOption;

    if ((pos = strLibretroOptions.find('|')) == std::string::npos)
    {
      strOption = strLibretroOptions;
      strLibretroOptions.clear();
    }
    else
    {
      strOption = strLibretroOptions.substr(0, pos);
      strLibretroOptions.erase(0, pos + 1);
    }

    vecOptions.push_back(strOption);
  }

  return vecOptions;
}
