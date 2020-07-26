/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "LibretroResources.h"
#include "audio/AudioStream.h"
#include "MemoryMap.h"
#include "settings/LibretroSettings.h"
#include "video/VideoStream.h"

#include <kodi/addon-instance/Game.h>

#include <memory>
#include <string>

class CGameLibRetro;

struct retro_game_geometry;
struct retro_memory_map_kodi;

namespace LIBRETRO
{
  class CClientBridge;
  class CLibretroDLL;

  class ATTRIBUTE_HIDDEN CLibretroEnvironment
  {
  public:
    static CLibretroEnvironment& Get(void);

    void Initialize(CGameLibRetro*                addon,
                    CLibretroDLL*                 client,
                    CClientBridge*                clientBridge);

    void Deinitialize(void);

    CGameLibRetro*                GetAddon(void)        { return m_addon; }
    CLibretroDLL*                 GetClient(void)       { return m_client; }
    CClientBridge*                GetClientBridge(void) { return m_clientBridge; }

    CVideoStream& Video(void) { return m_videoStream; }
    CAudioStream& Audio(void) { return m_audioStream; }

    void CloseStreams();

    void UpdateVideoGeometry(const retro_game_geometry &geometry);

    /*!
     * Returns the pixel format set by the libretro core. Instead of forwarding
     * this to the frontend, we store the value and report it on calls to
     * VideoRefresh().
     */
    GAME_PIXEL_FORMAT GetVideoFormat(void) const { return m_videoFormat; }

    GAME_VIDEO_ROTATION GetVideoRotation() const { return m_videoRotation; }

    /*!
     * Invoked when the frontend transfers a setting to the add-on.
     */
    void SetSetting(const std::string& name, const std::string& value);

    std::string GetResourcePath(const char* relPath);

    /*!
     * \brief Called after game has been run for a frame
     */
    void OnFrameEnd();

    bool EnvironmentCallback(unsigned cmd, void* data);

    const CMemoryMap& GetMemoryMap();

  private:
    CLibretroEnvironment(void);

    CGameLibRetro*                m_addon;
    CLibretroDLL*                 m_client;
    CClientBridge*                m_clientBridge;
    CVideoStream                  m_videoStream;
    CAudioStream                  m_audioStream;

    GAME_PIXEL_FORMAT   m_videoFormat;
    GAME_VIDEO_ROTATION m_videoRotation;

    CLibretroSettings m_settings;
    CLibretroResources m_resources;

    CMemoryMap m_mmap;
  };
} // namespace LIBRETRO
