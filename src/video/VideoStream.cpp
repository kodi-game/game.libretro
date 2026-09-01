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

#include <kodi/General.h>

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
  if (m_addon == nullptr || *m_geometry == geometry)
  {
    *m_geometry = geometry;
    return;
  }

  // A hardware rendering stream owns the context the core renders with, and the
  // core built its textures, shaders and framebuffers in that context when it
  // was told the context was ready. Closing the stream destroys the context and
  // takes all of that with it, while the core carries on using handles that no
  // longer refer to anything. So a geometry change never reopens one.
  if (m_streamType != GAME_STREAM_HW_FRAMEBUFFER)
  {
    CloseStream();
  }
  else
  {
    ReleaseOutgrownFramebuffer(geometry);
  }

  *m_geometry = geometry;
}

void CVideoStream::ReleaseOutgrownFramebuffer(const CVideoGeometry &geometry)
{
  if (!m_framebuffer)
    return;

  if (geometry.MaxWidth() <= m_framebufferWidth && geometry.MaxHeight() <= m_framebufferHeight)
    return;

  // Only SET_SYSTEM_AV_INFO arrives with a larger maximum, and libretro requires
  // the frontend to reinitialise in time for the callbacks the core makes later
  // in the same retro_run(). So the framebuffer is given up now, within the
  // frame, and the core asks for one at the new size when it next renders.
  kodi::Log(ADDON_LOG_DEBUG,
            "Maximum geometry grew to %ux%u, releasing the %ux%u framebuffer",
            geometry.MaxWidth(), geometry.MaxHeight(), m_framebufferWidth, m_framebufferHeight);

  m_stream.ReleaseBuffer(*m_framebuffer);
  m_framebuffer.reset();
  m_framebufferWidth = 0;
  m_framebufferHeight = 0;
}

bool CVideoStream::EnableHardwareRendering()
{
  if (m_addon == nullptr)
    return false;

  m_streamType = GAME_STREAM_HW_FRAMEBUFFER;

  return true;
}

void CVideoStream::DisableHardwareRendering()
{
  // Only ever undoes the line above, and only before a stream is opened: the
  // frontend is asked whether it can render in hardware after the stream has
  // been put in hardware mode, and a refusal has to put it back. Left set, the
  // stream would open a framebuffer the frontend has already said it cannot
  // provide, and close the game while the core was still starting up.
  if (m_streamType == GAME_STREAM_HW_FRAMEBUFFER && !m_stream.IsOpen())
    m_streamType = GAME_STREAM_UNKNOWN;
}

uintptr_t CVideoStream::GetHwFramebuffer()
{
  if (m_addon == nullptr)
    return 0;

  if (!m_stream.IsOpen() || m_streamType != GAME_STREAM_HW_FRAMEBUFFER)
    return 0;

  if (!m_framebuffer)
  {
    // libretro's get_current_framebuffer() takes no parameters, so the size
    // comes from the geometry the core reported. Use the maximum, as the
    // frontend has to hand back a framebuffer the core can render any frame
    // into without it being reallocated mid-game.
    const unsigned int width = m_geometry->MaxWidth();
    const unsigned int height = m_geometry->MaxHeight();

    if (width == 0 || height == 0)
    {
      kodi::Log(ADDON_LOG_ERROR, "Core reported no maximum geometry, cannot size framebuffer");
      return 0;
    }

    std::unique_ptr<game_stream_buffer> framebuffer(new game_stream_buffer{});

    // The frontend dispatches on the buffer type and rejects anything else, so
    // a zero-initialised buffer is always refused
    framebuffer->type = GAME_STREAM_HW_FRAMEBUFFER;

    // Don't cache a failed lookup, or the core would be stuck with a
    // framebuffer of 0 for the rest of the session. The frontend may not have
    // a renderer ready yet on the first few frames.
    if (!m_stream.GetBuffer(width, height, *framebuffer))
      return 0;

    if (framebuffer->hw_framebuffer.framebuffer == 0)
      return 0;

    m_framebuffer = std::move(framebuffer);
    m_framebufferWidth = width;
    m_framebufferHeight = height;
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
    properties.sw_framebuffer.nominal_display_aspect_ratio = m_geometry->DisplayAspectRatio();
    properties.sw_framebuffer.max_width = m_geometry->MaxWidth();
    properties.sw_framebuffer.max_height = m_geometry->MaxHeight();

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
    properties.video.nominal_display_aspect_ratio = m_geometry->DisplayAspectRatio();
    properties.video.max_width = m_geometry->MaxWidth();
    properties.video.max_height = m_geometry->MaxHeight();

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
    packet.video.display_aspect_ratio = m_geometry->DisplayAspectRatio();
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
    packet.sw_framebuffer.display_aspect_ratio = m_geometry->DisplayAspectRatio();
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

void CVideoStream::RenderHwFrame(unsigned int width, unsigned int height)
{
  if (m_addon == nullptr)
    return;

  if (!m_stream.IsOpen() || m_streamType != GAME_STREAM_HW_FRAMEBUFFER)
    return;

  // The buffer is released at the end of every frame, and a core that asks for
  // its framebuffer once -- in context_reset, keeping the ID because it never
  // changes -- has nothing held here by the time it presents. That is allowed:
  // libretro does not require get_current_framebuffer() to be called per frame.
  // Take one now rather than dropping the frame, which is what left Flycast
  // running with a black picture while Saturn, which does ask every frame,
  // rendered normally.
  if (!m_framebuffer)
    GetHwFramebuffer();

  if (!m_framebuffer)
    return;

  game_stream_packet packet{};

  packet.type = GAME_STREAM_HW_FRAMEBUFFER;
  packet.hw_framebuffer.framebuffer = m_framebuffer->hw_framebuffer.framebuffer;

  // The framebuffer is sized for the largest frame the core said it would draw,
  // and this frame is usually smaller. Without the size the frontend shows the
  // whole framebuffer, so the image sits in a corner of it.
#if defined(HAVE_GAME_HW_FRAMEBUFFER_SIZE)
  packet.hw_framebuffer.width = width;
  packet.hw_framebuffer.height = height;
#endif

  m_stream.AddData(packet);
}

void CVideoStream::OnFrameBegin()
{
  if (m_addon == nullptr)
    return;

  // Normally already open, from as soon as the core reported its geometry
  OpenHwStream();
}

bool CVideoStream::OpenHwStream()
{
  if (m_addon == nullptr)
    return false;

  if (!m_stream.IsOpen() && m_streamType == GAME_STREAM_HW_FRAMEBUFFER)
  {
    // The frontend allocates its framebuffer while opening the stream, so it
    // needs the frame size now. That is why this waits for the core's system
    // AV info, which was not available when hardware rendering was negotiated.
    //
    // It must not wait any longer than that, though. A core's GPU resources
    // come up in context_reset, and the frontend asks the core things that
    // depend on them -- its savestate size, for one -- before ever running a
    // frame. Leaving this until the first frame means answering those
    // questions with no renderer built yet.
    game_stream_properties streamProperties{GAME_STREAM_HW_FRAMEBUFFER};
#if defined(HAVE_GAME_HW_FRAMEBUFFER_SIZE)
    streamProperties.hw_framebuffer.max_width = m_geometry->MaxWidth();
    streamProperties.hw_framebuffer.max_height = m_geometry->MaxHeight();
#endif

    if (!m_stream.Open(streamProperties))
    {
      // This will stop the stream from trying to be opened twice
      m_streamType = GAME_STREAM_UNKNOWN;

      // The core was told hardware rendering was available when it asked, long
      // before the frontend tried and failed to build a context. It has already
      // wired itself up to render that way, and running it now means calling
      // through callbacks that were never installed.
      //
      // Closing the game from here does not stop that: this runs underneath the
      // frontend's own load, which carries on to set up controllers and run the
      // core regardless, and the core then faults with its context never reset.
      // Report the failure instead and let the caller fail the load.
      kodi::Log(ADDON_LOG_ERROR, "Failed to open the hardware rendering stream");
      return false;
    }
    else if (m_addon != nullptr)
    {
      // Tell the core its context is ready, now that the stream is open.
      // This cannot happen while the stream is being opened: cores ask for
      // their framebuffer from inside context_reset, and the stream has no
      // handle to ask through until Open() has returned.
      kodi::Log(ADDON_LOG_DEBUG, "Hardware rendering stream open, resetting core context");
      m_addon->HwContextReset();
    }
  }

  return true;
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
  m_framebufferWidth = 0;
  m_framebufferHeight = 0;
}

void CVideoStream::CloseStream()
{
  if (m_stream.IsOpen())
  {
    m_stream.Close();
    m_format = GAME_PIXEL_FORMAT_UNKNOWN;
  }

  // The cached framebuffer belongs to the stream that just closed. Drop it so
  // a reopened stream - after a geometry change, say - asks for a new one at
  // the new size instead of rendering into a stale framebuffer.
  m_framebuffer.reset();
  m_framebufferWidth = 0;
  m_framebufferHeight = 0;
}
