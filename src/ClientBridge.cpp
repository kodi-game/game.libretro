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

#include "ClientBridge.h"
#include "libretro.h"

// Causing errors with std::numeric_limits<int>::max()
#ifdef max
#undef max
#endif

#include <limits>

using namespace LIBRETRO;

CClientBridge::CClientBridge()
  : m_retro_keyboard_event(NULL),
    m_retro_disk_set_eject_state(NULL),
    m_retro_disk_get_eject_state(NULL),
    m_retro_disk_get_image_index(NULL),
    m_retro_disk_set_image_index(NULL),
    m_retro_disk_get_num_images(NULL),
    m_retro_disk_replace_image_index(NULL),
    m_retro_disk_add_image_index(NULL),
    m_retro_hw_context_reset(NULL),
    m_retro_hw_context_destroy(NULL),
    m_retro_audio_callback(NULL),
    m_retro_audio_set_state_callback(NULL),
    m_retro_frame_time_callback(NULL),
    m_retro_camera_initialized(NULL),
    m_retro_camera_deinitialized(NULL),
    m_retro_camera_frame_raw_buffer(NULL),
    m_retro_camera_frame_opengl_texture(NULL)
{
}

GAME_ERROR CClientBridge::KeyboardEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers)
{
  if (!m_retro_keyboard_event)
    return GAME_ERROR_FAILED;

  m_retro_keyboard_event(down, keycode, character, key_modifiers);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::DiskSetEjectState(GAME_EJECT_STATE ejected)
{
  if (!m_retro_disk_set_eject_state)
    return GAME_ERROR_FAILED;

  bool bSuccess = m_retro_disk_set_eject_state(ejected == GAME_EJECTED);

  return bSuccess ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_EJECT_STATE CClientBridge::DiskGetEjectState(void)
{
  if (!m_retro_disk_get_eject_state)
    return GAME_NOT_EJECTED;

  bool bEjected = m_retro_disk_get_eject_state();

  return bEjected ? GAME_EJECTED : GAME_NOT_EJECTED;
}

unsigned CClientBridge::DiskGetImageIndex(void)
{
  if (!m_retro_disk_get_image_index)
    return 0;

  return m_retro_disk_get_image_index();
}

GAME_ERROR CClientBridge::DiskSetImageIndex(unsigned index)
{
  if (!m_retro_disk_set_image_index)
    return GAME_ERROR_FAILED;

  if (index != GAME_NO_DISK)
    m_retro_disk_set_image_index(index);
  else
    m_retro_disk_set_image_index(std::numeric_limits<int>::max());

  return GAME_ERROR_NO_ERROR;
}

unsigned CClientBridge::DiskGetNumImages(void)
{
  if (!m_retro_disk_get_num_images)
    return 0;

  return m_retro_disk_get_num_images();
}

GAME_ERROR CClientBridge::DiskReplaceImageIndex(unsigned index, const char* url)
{
  if (!m_retro_disk_replace_image_index)
    return GAME_ERROR_FAILED;

  // Translate struct
  retro_game_info info = { };
  info.path = url;
  bool bSuccess = m_retro_disk_replace_image_index(index, &info);

  return bSuccess ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR CClientBridge::DiskAddImageIndex(void)
{
  if (!m_retro_disk_add_image_index)
    return GAME_ERROR_FAILED;

  bool bSuccess = m_retro_disk_add_image_index();

  return bSuccess ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
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

GAME_ERROR CClientBridge::AudioAvailable(void)
{
  if (!m_retro_audio_callback)
    return GAME_ERROR_FAILED;

  m_retro_audio_callback();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::AudioSetState(bool enabled)
{
  if (!m_retro_audio_set_state_callback)
    return GAME_ERROR_FAILED;

  m_retro_audio_set_state_callback(enabled);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::FrameTimeNotify(retro_usec_t usec)
{
  if (!m_retro_frame_time_callback)
    return GAME_ERROR_FAILED;

  m_retro_frame_time_callback(usec);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::CameraInitialized()
{
  if (!m_retro_camera_initialized)
    return GAME_ERROR_FAILED;

  m_retro_camera_initialized();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::CameraDeinitialized()
{
  if (!m_retro_camera_deinitialized)
    return GAME_ERROR_FAILED;

  m_retro_camera_deinitialized();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::CameraFrameRawBuffer(const uint32_t *buffer, unsigned width, unsigned height, size_t pitch)
{
  if (!m_retro_camera_frame_raw_buffer)
    return GAME_ERROR_FAILED;

  m_retro_camera_frame_raw_buffer(buffer, width, height, pitch);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CClientBridge::CameraFrameOpenglTexture(unsigned texture_id, unsigned texture_target, const float *affine)
{
  if (!m_retro_camera_frame_opengl_texture)
    return GAME_ERROR_FAILED;

  m_retro_camera_frame_opengl_texture(texture_id, texture_target, affine);

  return GAME_ERROR_NO_ERROR;
}
