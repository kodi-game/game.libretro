/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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
