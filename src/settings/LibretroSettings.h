/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "SettingsTypes.h"

#include <mutex>

#include <map>
#include <string>

class CGameLibRetro;
struct retro_variable;
struct retro_core_option_definition;
struct retro_core_option_v2_definition;

namespace LIBRETRO
{
  class CLibretroSettings
  {
  public:
    CLibretroSettings();

    void Initialize(CGameLibRetro* addon);
    void Deinitialize();

    bool Changed();
    void SetUnchanged();

    void SetAllSettings(const retro_variable* libretroVariables);

    /*!
     * \brief Take the settings a core declares through the core options API
     *
     * Same job as the retro_variable overload, for the two shapes the newer
     * API uses. A core that speaks this API never calls SET_VARIABLES at all,
     * so without these its settings are never registered and every value it
     * asks for comes back empty.
     */
    void SetAllSettings(const retro_core_option_definition* definitions);
    void SetAllSettings(const retro_core_option_v2_definition* definitions);

    const char* GetCurrentValue(const std::string& settingName);

    void SetCurrentValue(const std::string& name, const std::string& value);

  private:
    /*!
     * \brief Register one setting, checking it against what Kodi has
     */
    void AddSetting(CLibretroSetting setting, bool& bValid);

    /*!
     * \brief Generate settings and language files for Kodi
     */
    void GenerateSettings();

    // Frontend variables
    CGameLibRetro*                m_addon;
    std::string                   m_profileDirectory;

    // Settings variables
    LibretroSettings   m_settings;
    bool               m_bChanged;
    bool               m_bGenerated; // True if settings and language files have been generated
    std::mutex         m_mutex;
  };
} // namespace LIBRETRO
