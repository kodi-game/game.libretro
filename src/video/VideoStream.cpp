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

#include "VideoStream.h"
#include "VideoGeometry.h"
#include "libretro/LibretroEnvironment.h"

#include "libKODI_game.h"

using namespace LIBRETRO;

CVideoStream::CVideoStream() :
  m_frontend(nullptr),
  m_geometry(new CVideoGeometry)
{
}

void CVideoStream::Initialize(CHelper_libKODI_game* frontend)
{
  m_frontend = frontend;
}

void CVideoStream::Deinitialize()
{
  CloseStream();

  m_frontend = nullptr;
}

void CVideoStream::SetGeometry(const CVideoGeometry &geometry)
{
  // Close stream so it can be reopened with the updated geometry
  CloseStream();

  *m_geometry = geometry;
}

void CVideoStream::AddFrame(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, GAME_PIXEL_FORMAT format, GAME_VIDEO_ROTATION rotation)
{
  if (m_frontend == nullptr)
    return;

  if (m_format != format)
    CloseStream();

  if (m_stream == nullptr)
  {
    game_stream_properties properties{};

    properties.type = GAME_STREAM_VIDEO;
    properties.video.format = format;
    properties.video.nominal_width = m_geometry->NominalWidth();
    properties.video.nominal_height = m_geometry->NominalHeight();
    properties.video.max_width = m_geometry->MaxWidth();
    properties.video.max_height = m_geometry->MaxHeight();
    properties.video.aspect_ratio = m_geometry->AspectRatio();

    m_stream = m_frontend->OpenStream(properties);

    // Save format to detect unwanted changes
    m_format = format;
  }

  if (m_stream != nullptr)
  {
    game_stream_packet packet{};

    packet.type = GAME_STREAM_VIDEO;
    packet.video.width = width;
    packet.video.height = height;
    packet.video.rotation = rotation;
    packet.video.data = data;
    packet.video.size = size;

    m_frontend->AddStreamData(m_stream, packet);
  }
}

void CVideoStream::CloseStream()
{
  if (m_stream != nullptr)
  {
    m_frontend->CloseStream(m_stream);
    m_stream = nullptr;
    m_format = GAME_PIXEL_FORMAT_UNKNOWN;
  }
}
