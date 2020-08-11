/*
 *  Copyright (C) 2016-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "SingleFrameAudio.h"

#include <kodi/addon-instance/Game.h>

#include <stdint.h>

class CGameLibRetro;

namespace LIBRETRO
{
  class ATTRIBUTE_HIDDEN CAudioStream
  {
  public:
    CAudioStream();

    void Initialize(CGameLibRetro* addon);
    void Deinitialize();

    void AddFrame_S16NE(int16_t left, int16_t right) { m_singleFrameAudio.AddFrame(left, right); }

    void AddFrames_S16NE(const uint8_t* data, unsigned int size);

  private:
    CGameLibRetro*        m_addon;
    CSingleFrameAudio     m_singleFrameAudio;

    kodi::addon::CInstanceGame::CStream m_stream;
  };
}
