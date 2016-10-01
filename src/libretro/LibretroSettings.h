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
#pragma once

#include "LibretroSetting.h"

#include "p8-platform/threads/mutex.h"

#include <map>
#include <string>

namespace ADDON { class CHelper_libXBMC_addon; }

struct retro_variable;

namespace LIBRETRO
{
  class CLibretroSettings
  {
  public:
    CLibretroSettings();

    void Initialize(ADDON::CHelper_libXBMC_addon* addon);
    void Deinitialize();

    bool Changed();
    void SetUnchanged();

    void SetAllSettings(const retro_variable* libretroVariables);

    const char* GetCurrentValue(const std::string& settingName);

    void SetCurrentValue(const std::string& name, const std::string& value);

  private:
    // Frontend variables
    ADDON::CHelper_libXBMC_addon* m_addon;

    // Settings variables
    std::map<std::string, CLibretroSetting> m_settings;
    bool                                    m_bChanged;
    P8PLATFORM::CMutex                      m_mutex;
  };
} // namespace LIBRETRO
