/*
 *      Copyright (C) 2014 Team XBMC
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

#include "kodi/kodi_game_types.h"
#include "platform/threads/mutex.h"

#include <map>
#include <string>
#include <vector>

namespace ADDON { class CHelper_libXBMC_addon; }
class CHelper_libKODI_game;

namespace LIBRETRO
{
  class CClientBridge;
  class CLibretroDLL;

  class CLibretroEnvironment
  {
  public:
    static CLibretroEnvironment& Get(void);

    void Initialize(ADDON::CHelper_libXBMC_addon* xbmc, CHelper_libKODI_game* frontend, CLibretroDLL* client, CClientBridge* clientBridge);
    void Deinitialize(void);

    ADDON::CHelper_libXBMC_addon* GetXBMC(void)         { return m_xbmc; }
    CHelper_libKODI_game*         GetFrontend(void)     { return m_frontend; }
    CLibretroDLL*                 GetClient(void)       { return m_client; }
    CClientBridge*                GetClientBridge(void) { return m_clientBridge; }

    game_system_av_info GetSystemInfo(void) const { return m_systemInfo; }
    void UpdateSystemInfo(game_system_av_info info) { m_systemInfo = info; }

    /*!
     * Returns the pixel format set by the libretro core. Instead of forwarding
     * this to the frontend, we store the value and report it on calls to
     * VideoRefresh().
     */
    GAME_RENDER_FORMAT GetRenderFormat(void) const { return m_renderFormat; }

    /*!
     * Invoked when XBMC transfers a setting to the add-on.
     */
    void SetSetting(const char* name, const char* value);

    void AudioFrame(int16_t left, int16_t right);

    bool EnvironmentCallback(unsigned cmd, void* data);

  private:
    CLibretroEnvironment(void);

    ADDON::CHelper_libXBMC_addon* m_xbmc;
    CHelper_libKODI_game*         m_frontend;
    CLibretroDLL*                 m_client;
    CClientBridge*                m_clientBridge;

    game_system_av_info m_systemInfo;
    game_camera_info    m_cameraInfo;
    GAME_RENDER_FORMAT  m_renderFormat;

    std::map<std::string, std::vector<std::string> > m_variables; // Record the variables reported by libretro core (key -> values)
    std::map<std::string, std::string>               m_settings;  // Record the settings reported by XBMC (key -> current value)
    volatile bool                                    m_bSettingsChanged;
    PLATFORM::CMutex                                 m_settingsMutex;
  };
} // namespace LIBRETRO
