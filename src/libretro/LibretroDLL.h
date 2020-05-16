/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

struct AddonProps_Game;

namespace LIBRETRO
{
  class CLibretroDLL
  {
  public:
    CLibretroDLL(void);
    ~CLibretroDLL(void) { Unload(); }

    void Unload(void);
    bool Load(const std::string& gameClientDllPath);

    const std::string& GetPath() const { return m_strPath; }

    void     (*retro_set_environment)(retro_environment_t);
    void     (*retro_set_video_refresh)(retro_video_refresh_t);
    void     (*retro_set_audio_sample)(retro_audio_sample_t);
    void     (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t);
    void     (*retro_set_input_poll)(retro_input_poll_t);
    void     (*retro_set_input_state)(retro_input_state_t);
    void     (*retro_init)(void);
    void     (*retro_deinit)(void);
    unsigned (*retro_api_version)(void);
    void     (*retro_get_system_info)(struct retro_system_info *info);
    void     (*retro_get_system_av_info)(struct retro_system_av_info *info);
    void     (*retro_set_controller_port_device)(unsigned port, unsigned device);
    void     (*retro_reset)(void);
    void     (*retro_run)(void);
    size_t   (*retro_serialize_size)(void);
    bool     (*retro_serialize)(void *data, size_t size);
    bool     (*retro_unserialize)(const void *data, size_t size);
    void     (*retro_cheat_reset)(void);
    void     (*retro_cheat_set)(unsigned index, bool enabled, const char *code);
    bool     (*retro_load_game)(const struct retro_game_info *game);
    bool     (*retro_load_game_special)(unsigned game_type, const struct retro_game_info *info, size_t num_info);
    void     (*retro_unload_game)(void);
    unsigned (*retro_get_region)(void);
    void*    (*retro_get_memory_data)(unsigned id);
    size_t   (*retro_get_memory_size)(unsigned id);

  private:
    void* m_libretroClient;
    std::string m_strPath;
  };
} // namespace LIBRETRO
