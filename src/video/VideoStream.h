/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/addon-instance/Game.h>

#include <memory>
#include <stdint.h>

class CGameLibRetro;

namespace LIBRETRO
{
  class CVideoGeometry;

  class ATTR_DLL_LOCAL CVideoStream
  {
  public:
    CVideoStream();

    void Initialize(CGameLibRetro* addon);
    void Deinitialize();

    void SetGeometry(const CVideoGeometry &geometry);

    bool EnableHardwareRendering(const game_stream_hw_framebuffer_properties &properties);

    uintptr_t GetHwFramebuffer();
    bool GetSwFramebuffer(unsigned int width, unsigned int height, GAME_PIXEL_FORMAT requestedFormat, game_stream_sw_framebuffer_buffer &framebuffer);

    void AddFrame(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, GAME_PIXEL_FORMAT format, GAME_VIDEO_ROTATION rotation);
    void DupeFrame() { } // Not supported
    void RenderHwFrame();

    void OnFrameEnd();

  private:
    void CloseStream();

    // Initialization parameters
    CGameLibRetro* m_addon;

    // Stream properties
    kodi::addon::CInstanceGame::CStream m_stream;
    std::unique_ptr<CVideoGeometry> m_geometry;
    GAME_STREAM_TYPE m_streamType = GAME_STREAM_UNKNOWN;
    GAME_PIXEL_FORMAT m_format = GAME_PIXEL_FORMAT_UNKNOWN; // Guard against libretro changing formats
    std::unique_ptr<game_stream_buffer> m_framebuffer;
  };
}
