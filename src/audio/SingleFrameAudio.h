/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <stdint.h>
#include <vector>

namespace LIBRETRO
{
  class CAudioStream;

  class CSingleFrameAudio
  {
  public:
    CSingleFrameAudio(CAudioStream* audioStream);

    void AddFrame(int16_t left, int16_t right);

  private:
    CAudioStream* const  m_audioStream;
    std::vector<int16_t> m_data;
  };
}
