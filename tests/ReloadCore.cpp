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
retro_hw_render_callback hardware{};
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
  *info = {"Reload test", "1", "test", !state.memoryLoad, false};
}
void retro_get_system_av_info(retro_system_av_info* info)
{
  *info = state.hardware ? retro_system_av_info{{320, 240, 320, 240, 16.0f / 9.0f}, {70.0, 48000.0}}
                        : retro_system_av_info{{2, 2, 2, 2, 1.0f}, {70.0, 48000.0}};
}
void retro_set_controller_port_device(unsigned, unsigned) {}
void retro_reset() {}
void retro_run()
{
  if (loaded)
  {
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
    if (state.hardware)
    {
      ++state.frames;
      retro_game_geometry geometry{320, 240, 320, 240, state.frames % 2 ? 1.5f : 0.0f};
      environment(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
      unsigned rotation = (state.frames - 1) % 4;
      environment(RETRO_ENVIRONMENT_SET_ROTATION, &rotation);
      if (state.growGeometry)
      {
        hardware.get_current_framebuffer();
        const unsigned maximums[][2] = {{640, 480}, {320, 240}, {800, 480}, {800, 600}};
        if (state.frames <= 4)
        {
          retro_system_av_info info{geometry, {70.0, 48000.0}};
          info.geometry.max_width = maximums[state.frames - 1][0];
          info.geometry.max_height = maximums[state.frames - 1][1];
          environment(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &info);
        }
        else
        {
          geometry.max_width = 4096;
          geometry.max_height = 4096;
          environment(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
        }
        // Alternate explicit reacquisition with the FBO cached during context reset.
        state.geometryFramebuffer = state.frames % 2 ? hardware.get_current_framebuffer()
                                                      : state.resetFramebuffer;
      }
      video(RETRO_HW_FRAME_BUFFER_VALID, 320, 240, 0);
      return;
    }
    static const uint16_t pixels[] = {0x7c00, 0x03e0, 0x001f, 0x7fff};
    static const int16_t samples[] = {100, -100, 200, -200};
    video(pixels, 2, 2, 2 * sizeof(uint16_t));
    audio(samples, 2);
  }
}
size_t retro_serialize_size()
{
  if (state.ready && state.frames == 0)
    ++state.preframeSizeQueries;
  return state.ready ? 8 : 0;
}
bool retro_serialize(void*, size_t) { return false; }
bool retro_unserialize(const void*, size_t size)
{
  if (!state.ready || size != 8)
    return false;
  ++state.restores;
  return true;
}
void retro_cheat_reset() {}
void retro_cheat_set(unsigned, bool, const char*) {}
bool retro_load_game(const retro_game_info* info)
{
  if (loaded)
    return false;
  ++state.loadAttempts;
  if (state.memoryMaps && state.publishLoadMap)
  {
    state.memoryBuffer = 1;
    PublishMemoryMap();
  }
  if (info && info->data)
    ++state.memoryAttempts;
  if (state.probeBeforeLoad)
  {
    retro_hw_context_type preferred = RETRO_HW_CONTEXT_NONE;
    environment(RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER, &preferred);
  }
  if (state.hardware)
  {
    state.frames = 0;
    hardware = {};
    hardware.context_type = state.contextType;
    hardware.version_major = 3;
    hardware.version_minor = state.contextType == RETRO_HW_CONTEXT_OPENGL_CORE ? 2 : 0;
    hardware.depth = true;
    hardware.stencil = true;
    hardware.bottom_left_origin = true;
    hardware.context_reset = []
    {
      ++state.resets;
      state.resetFramebuffer = hardware.get_current_framebuffer();
      state.ready = state.resetFramebuffer == 42;
    };
    hardware.context_destroy = []
    {
      ++state.destroys;
      if (!loaded)
        ++state.destroysAfterUnload;
      state.ready = false;
    };
    if (state.failedLoads > 0)
    {
      hardware.context_reset = [] { ++state.failedCallbacks; };
      hardware.context_destroy = [] { ++state.failedCallbacks; };
    }
    if (!environment(RETRO_ENVIRONMENT_SET_HW_RENDER, &hardware))
      return false;
  }
  if (state.failedLoads > 0)
  {
    if (state.emitFailureFrame)
    {
      retro_system_av_info info{{2, 2, 2, 2, 1.0f}, {70.0, 48000.0}};
      environment(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &info);
      const uint16_t pixels[] = {0x7c00, 0x03e0, 0x001f, 0x7fff};
      const int16_t samples[] = {100, -100, 200, -200};
      video(pixels, 2, 2, 2 * sizeof(uint16_t));
      audio(samples, 2);
    }
    --state.failedLoads;
    if (state.softwareAfterFailure)
      state.hardware = false;
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
