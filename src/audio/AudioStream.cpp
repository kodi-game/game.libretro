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

#include "client.h"

using namespace LIBRETRO;

CAudioStream::CAudioStream() :
  m_addon(nullptr),
  m_singleFrameAudio(this)
{
}

void CAudioStream::Initialize(CGameLibRetro* addon)
{
  m_addon = addon;
}

void CAudioStream::Deinitialize()
{
  m_stream.Close();
  m_addon = nullptr;
}

void CAudioStream::AddFrames_S16NE(const uint8_t* data, unsigned int size)
{
  if (m_addon && !m_stream.IsOpen())
  {
    static const GAME_AUDIO_CHANNEL channelMap[] = { GAME_CH_FL, GAME_CH_FR, GAME_CH_NULL };

    game_stream_properties properties{};

    properties.type = GAME_STREAM_AUDIO;
    properties.audio.format = GAME_PCM_FORMAT_S16NE;
    properties.audio.channel_map = channelMap;

    if (m_stream.Open(properties))
      return;
  }

  game_stream_packet packet{};

  packet.type = GAME_STREAM_AUDIO;
  packet.audio.data = data;
  packet.audio.size = size;

  m_stream.AddData(packet);
}
