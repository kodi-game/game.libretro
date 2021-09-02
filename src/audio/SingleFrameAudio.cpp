/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SingleFrameAudio.h"
#include "AudioStream.h"

using namespace LIBRETRO;

#define FRAMES_PER_PACKET  100 // This can be calculated from CLibretroEnvironment::GetSystemInfo().timing.sample_rate
#define SAMPLES_PER_FRAME  2 // L + R
#define SAMPLE_SIZE        sizeof(int16_t)

CSingleFrameAudio::CSingleFrameAudio(CAudioStream* audioStream) :
  m_audioStream(audioStream)
{
  m_data.reserve(FRAMES_PER_PACKET * SAMPLES_PER_FRAME);
}

void CSingleFrameAudio::AddFrame(int16_t left, int16_t right)
{
  m_data.push_back(left);
  m_data.push_back(right);

  const unsigned int frameCount = static_cast<unsigned int>(m_data.size() / SAMPLES_PER_FRAME);
  if (frameCount >= FRAMES_PER_PACKET)
  {
    m_audioStream->AddFrames_S16NE(reinterpret_cast<const uint8_t*>(m_data.data()),
        static_cast<unsigned int>(m_data.size() * SAMPLE_SIZE));
    m_data.clear();
  }
}
