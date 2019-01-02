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

#include "ClientBridge.h"
#include "libretro.h"

// Causing errors with std::numeric_limits<int>::max()
#ifdef max
#undef max
#endif

#include <limits>

using namespace LIBRETRO;

CClientBridge::CClientBridge()
  : m_retro_keyboard_event(nullptr),
    m_retro_hw_context_reset(nullptr),
    m_retro_hw_context_destroy(nullptr),
    m_retro_audio_set_state_callback(nullptr),
    m_retro_audio_callback(nullptr),
    m_retro_frame_time_callback(nullptr)
{
}

GAME_ERROR CClientBridge::KeyboardEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers)
{
  if (!m_retro_keyboard_event)
    return GAME_ERROR_FAILED;

  m_retro_keyboard_event(down, keycode, character, key_modifiers);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::HwContextReset(void)
{
  if (!m_retro_hw_context_reset)
    return GAME_ERROR_FAILED;

  m_retro_hw_context_reset();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::HwContextDestroy(void)
{
  if (!m_retro_hw_context_destroy)
    return GAME_ERROR_FAILED;

  m_retro_hw_context_destroy();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::AudioEnable(bool enabled)
{
  if (!m_retro_audio_set_state_callback)
    return GAME_ERROR_FAILED;

  m_retro_audio_set_state_callback(enabled);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::AudioAvailable(void)
{
  if (!m_retro_audio_callback)
    return GAME_ERROR_FAILED;

  m_retro_audio_callback();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::FrameTime(int64_t usec)
{
  if (!m_retro_frame_time_callback)
    return GAME_ERROR_FAILED;

  m_retro_frame_time_callback(usec);

  return GAME_ERROR_NO_ERROR;
}
