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

#include "kodi_game_types.h"

#include <memory>

class CHelper_libKODI_game;

namespace LIBRETRO
{
  class CVideoGeometry;

  class CVideoStream
  {
  public:
    CVideoStream();

    void Initialize(CHelper_libKODI_game* frontend);
    void Deinitialize();

    void SetGeometry(const CVideoGeometry &geometry);

    void EnableHardwareRendering(const game_stream_hw_framebuffer_properties &properties);

    uintptr_t GetHwFramebuffer();
    bool GetSwFramebuffer(unsigned int width, unsigned int height, GAME_PIXEL_FORMAT requestedFormat, game_stream_sw_framebuffer_buffer &framebuffer);

    void AddFrame(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, GAME_PIXEL_FORMAT format, GAME_VIDEO_ROTATION rotation);
    void DupeFrame() { } // Not supported
    void RenderHwFrame();

    void OnFrameEnd();

  private:
    void CloseStream();

    // Initialization parameters
    CHelper_libKODI_game* m_frontend;

    // Stream properties
    void *m_stream = nullptr;
    std::unique_ptr<CVideoGeometry> m_geometry;
    GAME_STREAM_TYPE m_streamType = GAME_STREAM_UNKNOWN;
    GAME_PIXEL_FORMAT m_format = GAME_PIXEL_FORMAT_UNKNOWN; // Guard against libretro changing formats
    std::unique_ptr<game_stream_buffer> m_framebuffer;
  };
}
