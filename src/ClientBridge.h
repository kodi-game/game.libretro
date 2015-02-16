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

#include "xbmc_game_types.h"

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
    GAME_ERROR DiskSetEjectState(GAME_EJECT_STATE ejected);
    GAME_EJECT_STATE DiskGetEjectState(void);
    unsigned DiskGetImageIndex(void);
    GAME_ERROR DiskSetImageIndex(unsigned index);
    unsigned DiskGetNumImages(void);
    GAME_ERROR DiskReplaceImageIndex(unsigned index, const char* url);
    GAME_ERROR DiskAddImageIndex(void);
    GAME_ERROR HwContextReset(void);
    GAME_ERROR HwContextDestroy(void);
    GAME_ERROR AudioAvailable(void);
    GAME_ERROR AudioSetState(bool enabled);
    GAME_ERROR FrameTimeNotify(game_usec_t usec);
    GAME_ERROR CameraInitialized();
    GAME_ERROR CameraDeinitialized();
    GAME_ERROR CameraFrameRawBuffer(const uint32_t *buffer, unsigned width, unsigned height, size_t pitch);
    GAME_ERROR CameraFrameOpenglTexture(unsigned texture_id, unsigned texture_target, const float *affine);

    // The bridge is accomplished by invoking the callback provided by libretro's
    // enironment callback. The frontend can only invoke the commands above
    // if the client provides these callbacks at run-time.
    void (*m_retro_keyboard_event)(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers);
    bool (*m_retro_disk_set_eject_state)(bool ejected);
    bool (*m_retro_disk_get_eject_state)(void);
    unsigned (*m_retro_disk_get_image_index)(void);
    bool (*m_retro_disk_set_image_index)(unsigned index);
    unsigned (*m_retro_disk_get_num_images)(void);
    bool (*m_retro_disk_replace_image_index)(unsigned index, const struct retro_game_info *info);
    bool (*m_retro_disk_add_image_index)(void);
    void (*m_retro_hw_context_reset)(void);
    void (*m_retro_hw_context_destroy)(void);
    void (*m_retro_audio_callback)(void);
    void (*m_retro_audio_set_state_callback)(bool enabled);
    void (*m_retro_frame_time_callback)(retro_usec_t usec);
    void (*m_retro_camera_initialized)(void);
    void (*m_retro_camera_deinitialized)(void);
    void (*m_retro_camera_frame_raw_buffer)(const uint32_t *buffer, unsigned width, unsigned height, size_t pitch);
    void (*m_retro_camera_frame_opengl_texture)(unsigned texture_id, unsigned texture_target, const float *affine);
  };
} // namespace LIBRETRO
