/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro-common/libretro.h"

namespace LIBRETRO
{
  /*!
   * \brief The libretro microphone interface, captured from ALSA's default
   *        device on Linux
   */
  class CFrontendMicrophone
  {
  public:
    /*!
     * \brief Fill in the interface for RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE
     *
     * \return False on platforms without microphone support
     */
    static bool GetInterface(retro_microphone_interface& microphoneInterface);

    /*!
     * \brief Close any microphones the core left open
     */
    static void CloseAll();
  };
} // namespace LIBRETRO
