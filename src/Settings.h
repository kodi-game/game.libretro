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
#pragma once

#include "kodi/threads/mutex.h"

#include <map>
#include <string>
#include <vector>

namespace ADDON { class CHelper_libXBMC_addon; }
class CHelper_libKODI_game;

namespace LIBRETRO
{

class CSettings
{
public:
  static CSettings& Get(void);

  bool HasAddonSetting(const std::string& strKey) const;
  const char* GetAddonSetting(const std::string& strKey) const;
  void SetAddonSetting(const std::string& name, const std::string& value);

  bool HasOption(const std::string& setting, const std::string& option) const;
  void SetOptions(const std::string& setting, const std::vector<std::string>& options);

  bool IsChanged(void) const;
  void SetChanged(bool bChanged);

  static std::vector<std::string> ParseOptions(std::string strLibretroOptions);

private:
  CSettings(void);

  void FetchAddonSettings(const std::string& setting, const std::string& strDefault);

  std::map<std::string, std::vector<std::string> > m_options;           // setting -> possible values
  std::map<std::string, std::string>               m_addonSettings;     // setting -> addon's current value
  bool                                             m_bSettingsChanged;
  mutable PLATFORM::CMutex                         m_settingsMutex;
};

} // namespace LIBRETRO
