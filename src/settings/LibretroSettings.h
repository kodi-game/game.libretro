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

    const char* GetCurrentValue(const std::string& settingName);

    void SetCurrentValue(const std::string& name, const std::string& value);

  private:
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
