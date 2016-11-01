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
#pragma once

#include "SingleFrameAudio.h"

#include "kodi_game_types.h"

class CHelper_libKODI_game;

namespace LIBRETRO
{
  class CAudioStream
  {
  public:
    CAudioStream();

    void Initialize(CHelper_libKODI_game* frontend);
    void Deinitialize();

    void AddFrame_S16NE(int16_t left, int16_t right) { m_singleFrameAudio.AddFrame(left, right); }

    void AddFrames_S16NE(const uint8_t* data, unsigned int size);

  private:
    CHelper_libKODI_game* m_frontend;
    CSingleFrameAudio     m_singleFrameAudio;

    bool m_bAudioOpen;
  };
}
