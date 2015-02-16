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

#include "kodi/threads/mutex.h"
#include "kodi/xbmc_game_types.h"

#include <map>
#include <string>
#include <vector>

namespace ADDON { class CHelper_libXBMC_addon; }
class CHelper_libXBMC_game;

namespace LIBRETRO
{
  class CClientBridge;
  class CLibretroDLL;

  class CLibretroEnvironment
  {
  public:
    static void Initialize(ADDON::CHelper_libXBMC_addon* xbmc, CHelper_libXBMC_game* frontend, CLibretroDLL* client, CClientBridge* clientBridge);
    static void Deinitialize();
    static ADDON::CHelper_libXBMC_addon* GetXBMC() { return m_xbmc; }
    static CHelper_libXBMC_game* GetFrontend() { return m_frontend; }

    /*!
     * FPS info is used to calculate timing for toast messages and possibly
     * other things.
     */
    static void UpdateFramerate(double fps);

    /*!
     * Returns the pixel format set by the libretro core. Instead of forwarding
     * this to the frontend, we store the value and report it on calls to
     * VideoRefresh().
     */
    static GAME_PIXEL_FORMAT GetPixelFormat() { return m_pixelFormat; }

    /*!
     * Invoked when XBMC transfers a setting to the add-on.
     */
    static void SetSetting(const char* name, const char* value);

  private:
    static bool EnvironmentCallback(unsigned cmd, void *data);

    static ADDON::CHelper_libXBMC_addon* m_xbmc;
    static CHelper_libXBMC_game*         m_frontend;
    static CLibretroDLL*                 m_client;
    static CClientBridge*                m_clientBridge;

    static double            m_fps;
    static bool              m_bFramerateKnown; // true if UpdateFramerate() has been called
    static GAME_PIXEL_FORMAT m_pixelFormat;

    static std::map<std::string, std::vector<std::string> > m_variables; // Record the variables reported by libretro core (key -> values)
    static std::map<std::string, std::string>               m_settings;  // Record the settings reported by XBMC (key -> current value)
    static volatile bool                                    m_bSettingsChanged;
    static PLATFORM::CMutex                                 m_settingsMutex;
  };
} // namespace LIBRETRO
