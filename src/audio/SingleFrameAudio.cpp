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

#include "SingleFrameAudio.h"

#include "kodi/libKODI_game.h"

using namespace LIBRETRO;

#define FRAMES_PER_PACKET  100 // This can be calculated from CLibretroEnvironment::GetSystemInfo().timing.sample_rate
#define SAMPLES_PER_FRAME  2 // L + R
#define SAMPLE_SIZE        sizeof(int16_t)

CSingleFrameAudio::CSingleFrameAudio() :
  m_frontend(nullptr)
{
}

void CSingleFrameAudio::Initialize(CHelper_libKODI_game* frontend)
{
  m_frontend = frontend;
  m_data.reserve(FRAMES_PER_PACKET * SAMPLES_PER_FRAME);
}

void CSingleFrameAudio::Deinitialize()
{
  m_frontend = nullptr;
  m_data.clear();
}

void CSingleFrameAudio::AddFrame(int16_t left, int16_t right)
{
  m_data.push_back(left);
  m_data.push_back(right);

  const unsigned int frameCount = m_data.size() / SAMPLES_PER_FRAME;
  if (frameCount >= FRAMES_PER_PACKET)
  {
    if (m_frontend)
      m_frontend->AudioFrames(reinterpret_cast<const uint8_t*>(m_data.data()), m_data.size() * SAMPLE_SIZE, frameCount, GAME_AUDIO_FMT_S16NE);
    m_data.clear();
  }
}
