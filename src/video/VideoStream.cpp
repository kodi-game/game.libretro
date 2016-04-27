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
#include "libretro/LibretroEnvironment.h"

#include "kodi/libKODI_game.h"

using namespace LIBRETRO;

CVideoStream::CVideoStream() :
  m_frontend(nullptr),
  m_bVideoOpen(false),
  m_format(GAME_PIXEL_FORMAT_UNKNOWN),
  m_width(0),
  m_height(0)
{
}

void CVideoStream::Initialize(CHelper_libKODI_game* frontend)
{
  m_frontend = frontend;
  m_bVideoOpen = false;
  m_format = GAME_PIXEL_FORMAT_UNKNOWN;
  m_width = 0;
  m_height = 0;
}

void CVideoStream::Deinitialize()
{
  if (m_bVideoOpen)
    m_frontend->CloseStream(GAME_STREAM_VIDEO);

  m_frontend = nullptr;
  m_bVideoOpen = false;
}

void CVideoStream::AddFrame(const uint8_t* data, unsigned int size, unsigned int width, unsigned int height, GAME_PIXEL_FORMAT format)
{
  if (m_frontend)
  {
    if (m_format != format || m_width != width || m_height != height)
    {
      if (m_bVideoOpen)
      {
        m_frontend->CloseStream(GAME_STREAM_VIDEO);
        m_bVideoOpen = false;
      }

      if (m_frontend->OpenPixelStream(format, width, height))
      {
        m_bVideoOpen = true;
        m_format = format;
        m_width = width;
        m_height = height;
      }
    }
  }

  if (m_bVideoOpen)
    m_frontend->AddStreamData(GAME_STREAM_VIDEO, data, size);
}
