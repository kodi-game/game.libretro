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

#include "AudioStream.h"
#include "libretro/LibretroEnvironment.h"

#include "libKODI_game.h"

using namespace LIBRETRO;

CAudioStream::CAudioStream() :
  m_frontend(nullptr),
  m_singleFrameAudio(this)
{
}

void CAudioStream::Initialize(CHelper_libKODI_game* frontend)
{
  m_frontend = frontend;
}

void CAudioStream::Deinitialize()
{
  if (m_stream != nullptr)
  {
    m_frontend->CloseStream(m_stream);
    m_stream = nullptr;
  }

  m_frontend = nullptr;
}

void CAudioStream::AddFrames_S16NE(const uint8_t* data, unsigned int size)
{
  if (m_frontend && m_stream == nullptr)
  {
    static const GAME_AUDIO_CHANNEL channelMap[] = { GAME_CH_FL, GAME_CH_FR, GAME_CH_NULL };

    game_stream_properties properties{};

    properties.type = GAME_STREAM_AUDIO;
    properties.audio.format = GAME_PCM_FORMAT_S16NE;
    properties.audio.channel_map = channelMap;

    m_stream = m_frontend->OpenStream(properties);
  }

  if (m_stream != nullptr)
  {
    game_stream_packet packet{};

    packet.type = GAME_STREAM_AUDIO;
    packet.audio.data = data;
    packet.audio.size = size;

    m_frontend->AddStreamData(m_stream, packet);
  }
}
