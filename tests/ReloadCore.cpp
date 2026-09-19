/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include <libretro.h>

#include "ReloadCoreState.h"

namespace
{
retro_video_refresh_t video;
retro_audio_sample_batch_t audio;
retro_environment_t environment;
ReloadCoreState state;
bool loaded = false;
uint8_t memoryBuffers[3][16]{{0x11}, {0x22}, {0x33}};

void PublishMemoryMap(bool invalid = false)
{
  // Metadata may disappear as soon as the environment callback returns.
  char name[] = "RAM";
  retro_memory_descriptor descriptor{RETRO_MEMDESC_SYSTEM_RAM,
                                     memoryBuffers[state.memoryBuffer], 0, 0, 0, 0, 16, name};
  retro_memory_map map{invalid ? nullptr : &descriptor, 1};
  ++state.memoryMapCalls;
  if (environment(RETRO_ENVIRONMENT_SET_MEMORY_MAPS, &map))
    ++state.acceptedMemoryMaps;
  else
    ++state.rejectedMemoryMaps;
  name[0] = 'X';
}
}

extern "C"
{
ReloadCoreState* test_core_state() { return &state; }
bool test_core_environment(unsigned command, void* data) { return environment(command, data); }
void retro_set_environment(retro_environment_t callback) { environment = callback; }
void retro_set_video_refresh(retro_video_refresh_t callback) { video = callback; }
void retro_set_audio_sample(retro_audio_sample_t) {}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t callback) { audio = callback; }
void retro_set_input_poll(retro_input_poll_t) {}
void retro_set_input_state(retro_input_state_t) {}
void retro_init()
{
  ++state.initializations;
  if (state.memoryMaps)
  {
    state.memoryBuffer = 0;
    PublishMemoryMap();
  }
}
void retro_deinit()
{
  ++state.deinitializations;
  state.deinitializedWithContent = loaded;
}
unsigned retro_api_version() { return RETRO_API_VERSION; }
void retro_get_system_info(retro_system_info* info)
{
  *info = {"Reload test", "1", "test", true, false};
}
void retro_get_system_av_info(retro_system_av_info* info)
{
  *info = {{2, 2, 2, 2, 1.0f}, {70.0, 48000.0}};
}
void retro_set_controller_port_device(unsigned, unsigned) {}
void retro_reset() {}
void retro_run()
{
  if (loaded)
  {
    ++state.frames;
    if (state.replaceMapOnRun)
    {
      state.replaceMapOnRun = false;
      state.memoryBuffer = 2;
      PublishMemoryMap();
      ++state.runtimeReplacements;
    }
    if (state.invalidMapOnRun)
    {
      state.invalidMapOnRun = false;
      PublishMemoryMap(true);
    }
    static const uint16_t pixels[] = {0x7c00, 0x03e0, 0x001f, 0x7fff};
    static const int16_t samples[] = {100, -100, 200, -200};
    video(pixels, 2, 2, 2 * sizeof(uint16_t));
    audio(samples, 2);
  }
}
size_t retro_serialize_size() { return 0; }
bool retro_serialize(void*, size_t) { return false; }
bool retro_unserialize(const void*, size_t) { return false; }
void retro_cheat_reset() {}
void retro_cheat_set(unsigned, bool, const char*) {}
bool retro_load_game(const retro_game_info*)
{
  if (loaded)
    return false;
  ++state.loadAttempts;
  if (state.memoryMaps && state.publishLoadMap)
  {
    state.memoryBuffer = 1;
    PublishMemoryMap();
  }
  if (state.failedLoads > 0)
  {
    --state.failedLoads;
    return false;
  }
  loaded = true;
  return true;
}
bool retro_load_game_special(unsigned, const retro_game_info*, size_t) { return false; }
void retro_unload_game()
{
  ++state.unloads;
  loaded = false;
}
unsigned retro_get_region() { return RETRO_REGION_NTSC; }
void* retro_get_memory_data(unsigned type)
{
  if (!loaded)
    ++state.memoryQueriesAfterUnload;
  return loaded && state.memoryMaps && type == RETRO_MEMORY_SYSTEM_RAM
             ? memoryBuffers[state.memoryBuffer] : nullptr;
}
size_t retro_get_memory_size(unsigned type)
{
  if (!loaded)
    ++state.memoryQueriesAfterUnload;
  return loaded && state.memoryMaps && type == RETRO_MEMORY_SYSTEM_RAM ? 16 : 0;
}
}
