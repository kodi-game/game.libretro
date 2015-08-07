/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "kodi/kodi_game_types.h"

struct retro_game_info;

// from libretro.h
typedef int64_t retro_usec_t;

namespace LIBRETRO
{
  /*!
   * Provide a bridge for client callbacks that we want to expose to the
   * frontend as part of the API. Colloquially, it fixes the callbacks of
   * callbacks shit.
   */
  class CClientBridge
  {
  public:
    CClientBridge();

    // These functions are invoked when the frontend calls the XBMC game API.
    // They do not have public methods exposed for them in the libretro API.
    // To maintain ABI, they were exposed through the generic EnvironmentCallback()
    // function in a tangled mess of callbacks for callbacks.
    GAME_ERROR KeyboardEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
    GAME_ERROR HwContextReset(void);
    GAME_ERROR HwContextDestroy(void);
    GAME_ERROR AudioAvailable(void);
    GAME_ERROR AudioSetState(bool enabled);

    // The bridge is accomplished by invoking the callback provided by libretro's
    // enironment callback. The frontend can only invoke the commands above
    // if the client provides these callbacks at run-time.
    void (*m_retro_keyboard_event)(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
    void (*m_retro_hw_context_reset)(void);
    void (*m_retro_hw_context_destroy)(void);
    void (*m_retro_audio_callback)(void);
    void (*m_retro_audio_set_state_callback)(bool enabled);
  };
} // namespace LIBRETRO
