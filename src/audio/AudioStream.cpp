/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "AudioStream.h"
#include "libretro/LibretroEnvironment.h"
#include "log/Log.h"

#include "client.h"

using namespace LIBRETRO;

CAudioStream::CAudioStream() :
  m_addon(nullptr),
  m_singleFrameAudio(this)
{
}

void CAudioStream::Initialize(CGameLibRetro* addon)
{
  m_bLoggedOpenFailure = false;
  m_addon = addon;
}

void CAudioStream::Deinitialize()
{
  m_stream.Close();
  m_bLoggedOpenFailure = false;
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

    // Opening the stream used to return, throwing away the frames that
    // prompted it. Fall through and play them instead.
    if (!m_stream.Open(properties))
    {
      // Said once rather than per frame: a core delivering audio into a stream
      // that will not open is silent, and otherwise looks the same from the log
      // as a core that never delivers any.
      if (!m_bLoggedOpenFailure)
      {
        m_bLoggedOpenFailure = true;
        esyslog("Failed to open the audio stream, this game will be silent");
      }
      return;
    }

    m_bLoggedOpenFailure = false;
  }

  game_stream_packet packet{};

  packet.type = GAME_STREAM_AUDIO;
  packet.audio.data = data;
  packet.audio.size = size;

  m_stream.AddData(packet);
}
