/*
 *      Copyright (C) 2014-2016 Team Kodi
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

#include <kodi/addon-instance/Game.h>

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

    // These functions are invoked when the frontend calls the Kodi game API.
    // They do not have public methods exposed for them in the libretro API.
    // To maintain ABI, they were exposed through the generic EnvironmentCallback()
    // function in a tangled mess of callbacks for callbacks.
    GAME_ERROR KeyboardEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
    GAME_ERROR HwContextReset(void);
    GAME_ERROR HwContextDestroy(void);
    GAME_ERROR AudioEnable(bool enabled);
    GAME_ERROR AudioAvailable(void);
    GAME_ERROR FrameTime(int64_t);

    typedef void (*KeyboardEventCallback)(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
    typedef void (*HwContextResetCallback)(void);
    typedef void (*HwContextDestroyCallback)(void);
    typedef void (*AudioEnableCallback)(bool enabled);
    typedef void (*AudioAvailableCallback)(void);
    typedef void (*FrameTimeCallback)(int64_t);

    void SetKeyboardEvent(KeyboardEventCallback callback)       { m_retro_keyboard_event = callback; }
    void SetHwContextReset(HwContextResetCallback callback)     { m_retro_hw_context_reset = callback; }
    void SetHwContextDestroy(HwContextDestroyCallback callback) { m_retro_hw_context_destroy = callback; }
    void SetAudioEnable(AudioEnableCallback callback)           { m_retro_audio_set_state_callback = callback; }
    void SetAudioAvailable(AudioAvailableCallback callback)     { m_retro_audio_callback = callback; }
    void SetFrameTime(FrameTimeCallback callback)               { m_retro_frame_time_callback = callback; }

  private:
    // The bridge is accomplished by invoking the callback provided by libretro's
    // enironment callback. The frontend can only invoke the commands above
    // if the client provides these callbacks at run-time.
    KeyboardEventCallback    m_retro_keyboard_event;
    HwContextResetCallback   m_retro_hw_context_reset;
    HwContextDestroyCallback m_retro_hw_context_destroy;
    AudioEnableCallback      m_retro_audio_set_state_callback;
    AudioAvailableCallback   m_retro_audio_callback;
    FrameTimeCallback        m_retro_frame_time_callback;
  };
} // namespace LIBRETRO
