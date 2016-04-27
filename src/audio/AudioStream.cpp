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

#include "kodi/libKODI_game.h"

using namespace LIBRETRO;

CAudioStream::CAudioStream() :
  m_frontend(nullptr),
  m_singleFrameAudio(this),
  m_bAudioOpen(false)
{
}

void CAudioStream::Initialize(CHelper_libKODI_game* frontend)
{
  m_frontend = frontend;
  m_bAudioOpen = false;
}

void CAudioStream::Deinitialize()
{
  if (m_bAudioOpen)
    m_frontend->CloseStream(GAME_STREAM_AUDIO);

  m_frontend = nullptr;
  m_bAudioOpen = false;
}

void CAudioStream::AddFrames_S16NE(const uint8_t* data, unsigned int size)
{
  if (m_frontend && !m_bAudioOpen)
  {
    const double samplerate = CLibretroEnvironment::Get().GetSystemInfo().timing.sample_rate;

    if (samplerate)
    {
      if (m_frontend->OpenPCMStream(GAME_PCM_FORMAT_S16NE, static_cast<unsigned int>(samplerate + 0.5), CH_FL_FR))
        m_bAudioOpen = true;
    }
  }

  if (m_bAudioOpen)
    m_frontend->AddStreamData(GAME_STREAM_AUDIO, data, size);
}
