/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "VideoStream.h"
#include "VideoGeometry.h"
#include "libretro/LibretroEnvironment.h"

#include "client.h"

using namespace LIBRETRO;

CVideoStream::CVideoStream() :
  m_addon(nullptr),
  m_geometry(new CVideoGeometry)
{
}

void CVideoStream::Initialize(CGameLibRetro* addon)
{
  m_addon = addon;
}

void CVideoStream::Deinitialize()
{
  if (m_addon == nullptr)
    return;

  CloseStream();

  m_addon = nullptr;
}

void CVideoStream::SetGeometry(const CVideoGeometry &geometry)
{
  // Close stream so it can be reopened with the updated geometry
  if (m_addon != nullptr)
    CloseStream();

  *m_geometry = geometry;
}

bool CVideoStream::EnableHardwareRendering(const game_stream_hw_framebuffer_properties &properties)
{
  if (m_addon == nullptr)
    return false;

  CloseStream();

  game_stream_properties streamProps{};

  streamProps.type = GAME_STREAM_HW_FRAMEBUFFER;
  streamProps.hw_framebuffer = properties;

  if (!m_stream.Open(streamProps))
    return false;

  m_streamType = GAME_STREAM_HW_FRAMEBUFFER;

  return true;
}

uintptr_t CVideoStream::GetHwFramebuffer()
{
  if (m_addon == nullptr)
    return 0;

  if (!m_stream.IsOpen() || m_streamType != GAME_STREAM_HW_FRAMEBUFFER)
    return 0;

  if (!m_framebuffer)
  {
    m_framebuffer.reset(new game_stream_buffer{});

    if (!m_stream.GetBuffer(0, 0, *m_framebuffer))
      return 0;
  }

  return m_framebuffer->hw_framebuffer.framebuffer;
}

bool CVideoStream::GetSwFramebuffer(unsigned int width, unsigned int height, GAME_PIXEL_FORMAT requestedFormat, game_stream_sw_framebuffer_buffer &framebuffer)
{
  if (m_addon == nullptr)
    return false;

  if (!m_stream.IsOpen())
  {
    game_stream_properties properties{};

    properties.type = GAME_STREAM_SW_FRAMEBUFFER;
    properties.sw_framebuffer.format = requestedFormat;
    properties.sw_framebuffer.nominal_width = m_geometry->NominalWidth();
    properties.sw_framebuffer.nominal_height = m_geometry->NominalHeight();
    properties.sw_framebuffer.max_width = m_geometry->MaxWidth();
    properties.sw_framebuffer.max_height = m_geometry->MaxHeight();
    properties.sw_framebuffer.aspect_ratio = m_geometry->AspectRatio();

    m_stream.Open(properties);
    m_streamType = GAME_STREAM_SW_FRAMEBUFFER;
  }

  if (!m_stream.IsOpen() || m_streamType != GAME_STREAM_SW_FRAMEBUFFER)
    return false;

  if (m_framebuffer != nullptr)
  {
    m_stream.ReleaseBuffer(*m_framebuffer);
  }

  m_framebuffer.reset(new game_stream_buffer{});

  if (!m_stream.GetBuffer(width, height, *m_framebuffer))
    return false;

  framebuffer = m_framebuffer->sw_framebuffer;

  return true;
}

void CVideoStream::AddFrame(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, GAME_PIXEL_FORMAT format, GAME_VIDEO_ROTATION rotation)
{
  if (m_addon == nullptr)
    return;

  // Only care if format changes for video stream
  if (m_streamType == GAME_STREAM_VIDEO)
  {
    if (m_format != format)
    {
      // Close stream so it can be reopened with the updated format
      CloseStream();
    }
  }

  if (!m_stream.IsOpen())
  {
    game_stream_properties properties{};

    properties.type = GAME_STREAM_VIDEO;
    properties.video.format = format;
    properties.video.nominal_width = m_geometry->NominalWidth();
    properties.video.nominal_height = m_geometry->NominalHeight();
    properties.video.max_width = m_geometry->MaxWidth();
    properties.video.max_height = m_geometry->MaxHeight();
    properties.video.aspect_ratio = m_geometry->AspectRatio();

    m_stream.Open(properties);
    m_streamType = GAME_STREAM_VIDEO;

    // Save format to detect unwanted changes
    m_format = format;
  }

  if (!m_stream.IsOpen())
    return;

  game_stream_packet packet{};

  switch (m_streamType)
  {
  case GAME_STREAM_VIDEO:
  {
    packet.type = GAME_STREAM_VIDEO;
    packet.video.width = width;
    packet.video.height = height;
    packet.video.rotation = rotation;
    packet.video.data = data;
    packet.video.size = size;
    break;
  }
  case GAME_STREAM_SW_FRAMEBUFFER:
  {
    packet.type = GAME_STREAM_SW_FRAMEBUFFER;
    packet.sw_framebuffer.width = width;
    packet.sw_framebuffer.height = height;
    packet.sw_framebuffer.rotation = rotation;
    packet.sw_framebuffer.data = data;
    packet.sw_framebuffer.size = size;
    break;
  }
  default:
    return;
  }

  m_stream.AddData(packet);
}

void CVideoStream::RenderHwFrame()
{
  if (m_addon == nullptr)
    return;

  if (!m_stream.IsOpen() || m_streamType != GAME_STREAM_HW_FRAMEBUFFER)
    return;

  if (!m_framebuffer)
    return;

  game_stream_packet packet{};

  packet.type = GAME_STREAM_HW_FRAMEBUFFER;
  packet.hw_framebuffer.framebuffer = m_framebuffer->hw_framebuffer.framebuffer;

  m_stream.AddData(packet);
}

void CVideoStream::OnFrameEnd()
{
  if (m_addon == nullptr)
    return;

  if (!m_stream.IsOpen())
    return;

  if (!m_framebuffer)
    return;

  m_stream.ReleaseBuffer(*m_framebuffer);
  m_framebuffer.reset();
}

void CVideoStream::CloseStream()
{
  if (m_stream.IsOpen())
  {
    m_stream.Close();
    m_format = GAME_PIXEL_FORMAT_UNKNOWN;
  }
}
