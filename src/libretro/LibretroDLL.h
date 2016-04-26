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

#include "libretro.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace ADDON { class CHelper_libXBMC_addon; }

struct game_client_properties;

namespace LIBRETRO
{
  class CLibretroDLL
  {
  public:
    CLibretroDLL(ADDON::CHelper_libXBMC_addon* xbmc);
    ~CLibretroDLL(void) { Unload(); }

    void Unload(void);
    bool Load(const game_client_properties* gameClientProps);

    const std::string& GetPath() const { return m_strPath; }
    const std::string& GetSystemDirectory() const { return m_strSystemDirectory; }
    const std::string& GetContentDirectory() const { return m_strContentDirectory; }
    const std::string& GetSaveDirectory() const { return m_strSaveDirectory; }

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
    ADDON::CHelper_libXBMC_addon* m_xbmc;
    void*                         m_libretroClient;
    std::string                   m_strPath;
    std::string                   m_strSystemDirectory;
    std::string                   m_strContentDirectory;
    std::string                   m_strSaveDirectory;
  };
} // namespace LIBRETRO
