/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "SingleFrameAudio.h"

#include <kodi/addon-instance/Game.h>

#include <mutex>
#include <stdint.h>

class CGameLibRetro;

namespace LIBRETRO
{
  class ATTR_DLL_LOCAL CAudioStream
  {
  public:
    CAudioStream();

    void Initialize(CGameLibRetro* addon);
    void Deinitialize();
    void CloseStream();

    void AddFrame_S16NE(int16_t left, int16_t right);

    void AddFrames_S16NE(const uint8_t* data, unsigned int size);

  private:
    CGameLibRetro*        m_addon;
    CSingleFrameAudio     m_singleFrameAudio;

    kodi::addon::CInstanceGame::CStream m_stream;

    // A core may deliver audio from its own thread, which can still be running
    // while the stream is closed. LRPS2 does.
    std::recursive_mutex m_mutex;

    //! \brief Set once a failed stream open has been reported
    bool m_bLoggedOpenFailure{false};
  };
}
