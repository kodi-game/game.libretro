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

#include "SettingsTypes.h"

#include "p8-platform/threads/mutex.h"

#include <map>
#include <string>

namespace ADDON { class CHelper_libXBMC_addon; }

struct game_client_properties;
struct retro_variable;

namespace LIBRETRO
{
  class CLibretroSettings
  {
  public:
    CLibretroSettings();

    void Initialize(ADDON::CHelper_libXBMC_addon* addon, const game_client_properties* props);
    void Deinitialize();

    bool Changed();
    void SetUnchanged();

    void SetAllSettings(const retro_variable* libretroVariables);

    const char* GetCurrentValue(const std::string& settingName);

    void SetCurrentValue(const std::string& name, const std::string& value);

  private:
    /*!
     * \brief Generate settings and language files for Kodi
     */
    void GenerateSettings();

    // Frontend variables
    ADDON::CHelper_libXBMC_addon* m_addon;
    std::string                   m_profileDirectory;

    // Settings variables
    LibretroSettings   m_settings;
    bool               m_bChanged;
    bool               m_bGenerated; // True if settings and language files have been generated
    P8PLATFORM::CMutex m_mutex;
  };
} // namespace LIBRETRO
