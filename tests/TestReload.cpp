/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include <kodi/c-api/addon_base.h>
#include <kodi/c-api/addon-instance/game.h>
#include <kodi/c-api/filesystem.h>
#include <kodi/c-api/network.h>
#include <kodi/versions.h>

#include "ReloadCoreState.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <map>
#include <utility>
#include <vector>

namespace
{
struct StreamState
{
  GAME_STREAM_TYPE type;
  bool resetAttempted{false};
  bool destroyNotified{false};
};

std::map<KODI_GAME_STREAM_HANDLE, StreamState> streams;
unsigned videoPackets = 0;
unsigned audioPackets = 0;
unsigned hardwarePackets = 0;
unsigned hardwareOpens = 0;
unsigned hardwareStarts = 0;
unsigned hardwareCloses = 0;
unsigned framebufferQueries = 0;
bool geometryGrowth = false;
std::vector<std::pair<unsigned, unsigned>> framebufferSizes;
bool failOpen = false;
bool failStart = false;
bool failFramebuffer = false;
bool zeroFramebuffer = false;
bool contextCurrent = false;
AddonInstance_Game* currentGame = nullptr;
enum class HardwareBackend { OpenGL, OpenGLES, None };
HardwareBackend backend = HardwareBackend::OpenGL;
game_hw_rendering_properties negotiated{};
bool hardwareRefused = false;
unsigned negotiations = 0;

void Require(bool condition, const char* message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(EXIT_FAILURE);
  }
}

KODI_GAME_STREAM_HANDLE OpenStream(KODI_HANDLE, const game_stream_properties* properties)
{
  if (properties->type == GAME_STREAM_HW_FRAMEBUFFER)
  {
    Require(negotiated.context_type != GAME_HW_CONTEXT_NONE,
            "hardware stream opened without an accepted context");
    ++hardwareOpens;
    Require(properties->hw_framebuffer.max_width == 320 &&
                properties->hw_framebuffer.max_height == 240,
            "incorrect hardware allocation geometry");
    Require(properties->hw_framebuffer.nominal_display_aspect_ratio == 16.0f / 9.0f,
            "nominal hardware display aspect ratio lost");
    if (failOpen)
      return nullptr;
  }
  auto* handle = new int;
  streams.emplace(handle, StreamState{properties->type});
  if (properties->type == GAME_STREAM_VIDEO || properties->type == GAME_STREAM_SW_FRAMEBUFFER)
    hardwareRefused = false;
  return handle;
}

bool EnableHardwareRendering(KODI_HANDLE, const game_hw_rendering_properties* properties)
{
  ++negotiations;
  for (const auto& stream : streams)
  {
    if (stream.second.type == GAME_STREAM_HW_FRAMEBUFFER)
      return false;
  }
  negotiated = {};
  hardwareRefused = false;
  if (properties->context_type == GAME_HW_CONTEXT_NONE)
    return false;
  const bool supported =
      (backend == HardwareBackend::OpenGL &&
       (properties->context_type == GAME_HW_CONTEXT_OPENGL ||
        properties->context_type == GAME_HW_CONTEXT_OPENGL_CORE)) ||
      (backend == HardwareBackend::OpenGLES &&
       (properties->context_type == GAME_HW_CONTEXT_OPENGLES2 ||
        properties->context_type == GAME_HW_CONTEXT_OPENGLES3 ||
        properties->context_type == GAME_HW_CONTEXT_OPENGLES_VERSION));
  if (!supported || (properties->context_type == GAME_HW_CONTEXT_OPENGL_CORE &&
                     properties->version_major == 0))
  {
    hardwareRefused = true;
    return false;
  }
  negotiated = *properties;
  return true;
}

void DestroyHardwareContext(KODI_GAME_STREAM_HANDLE handle)
{
  auto& stream = streams.at(handle);
  if (stream.type != GAME_STREAM_HW_FRAMEBUFFER || !stream.resetAttempted || stream.destroyNotified)
    return;

  stream.destroyNotified = true;
  const bool previousContext = contextCurrent;
  contextCurrent = true;
  Require(currentGame->toAddon->HwContextDestroy(currentGame) == GAME_ERROR_NO_ERROR,
          "hardware context destroy failed");
  contextCurrent = previousContext;
}

void DestroyHardwareContexts()
{
  for (const auto& stream : streams)
    DestroyHardwareContext(stream.first);
}

void CloseStream(KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle)
{
  if (streams.at(handle).type == GAME_STREAM_HW_FRAMEBUFFER)
    ++hardwareCloses;
  DestroyHardwareContext(handle);
  Require(streams.erase(handle) == 1, "closing an unknown stream");
  delete static_cast<int*>(handle);
}

bool StartStream(KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle)
{
  Require(streams.at(handle).type == GAME_STREAM_HW_FRAMEBUFFER, "starting an invalid hardware stream");
  streams.at(handle).resetAttempted = true;
  ++hardwareStarts;
  contextCurrent = true;
  const auto result = currentGame->toAddon->HwContextReset(currentGame);
  contextCurrent = false;
  return result == GAME_ERROR_NO_ERROR && !failStart;
}

bool GetStreamBuffer(KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle,
                     unsigned width, unsigned height, game_stream_buffer* buffer)
{
  Require(streams.at(handle).type == GAME_STREAM_HW_FRAMEBUFFER, "framebuffer requested without handle");
  Require(contextCurrent, "framebuffer requested without current context");
  if (!geometryGrowth)
    Require(width == 320 && height == 240, "incorrect framebuffer request geometry");
  framebufferSizes.emplace_back(width, height);
  Require(buffer->type == GAME_STREAM_HW_FRAMEBUFFER, "incorrect framebuffer request type");
  ++framebufferQueries;
  if (failFramebuffer)
    return false;
  buffer->hw_framebuffer.framebuffer = zeroFramebuffer ? 0 : 42;
  return true;
}

void AddStreamData(KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle, const game_stream_packet* packet)
{
  Require(streams.count(handle) == 1, "packet sent to a closed stream");
  Require(streams.at(handle).type == packet->type, "packet type differs from stream type");
  if (packet->type == GAME_STREAM_VIDEO)
  {
    static const uint16_t expected[] = {0x7c00, 0x03e0, 0x001f, 0x7fff};
    Require(packet->video.width == 2 && packet->video.height == 2, "incorrect frame geometry");
    Require(packet->video.size == sizeof(expected), "incorrect video packet size");
    Require(std::memcmp(packet->video.data, expected, sizeof(expected)) == 0, "incorrect pixels");
    ++videoPackets;
  }
  else if (packet->type == GAME_STREAM_AUDIO)
  {
    static const int16_t expected[] = {100, -100, 200, -200};
    Require(packet->audio.size == sizeof(expected), "incorrect audio packet size");
    Require(std::memcmp(packet->audio.data, expected, sizeof(expected)) == 0, "incorrect samples");
    ++audioPackets;
  }
  else if (packet->type == GAME_STREAM_HW_FRAMEBUFFER)
  {
    Require(packet->hw_framebuffer.framebuffer == 42, "incorrect submitted framebuffer");
    Require(packet->hw_framebuffer.width == 320 && packet->hw_framebuffer.height == 240,
            "incorrect hardware frame dimensions");
    Require(packet->hw_framebuffer.display_aspect_ratio == (hardwarePackets % 2 ? 0.0f : 1.5f),
            "current hardware display aspect ratio lost");
    const GAME_VIDEO_ROTATION rotations[] = {
        GAME_VIDEO_ROTATION_0, GAME_VIDEO_ROTATION_90_CCW,
        GAME_VIDEO_ROTATION_180_CCW, GAME_VIDEO_ROTATION_270_CCW};
    Require(packet->hw_framebuffer.rotation == rotations[hardwarePackets % 4],
            "current hardware rotation lost");
    ++hardwarePackets;
  }
  else
    Require(false, "unexpected stream type");
}
}

int main(int argc, char** argv)
{
  Require(argc == 3 || argc == 4,
          "usage: test_reload <wrapper library> <fixture core library> [scenario]");
  const char* scenario = argc == 4 ? argv[3] : "software";
  const bool testHardware = std::strcmp(scenario, "hardware") == 0;
  const bool testPreference = std::strncmp(scenario, "preferred_", 10) == 0;
  const bool testHandoff = std::strcmp(scenario, "retained_hardware_software") == 0;
  const bool testGrowth = std::strcmp(scenario, "hardware_geometry_growth") == 0;
  if (std::strcmp(scenario, "preferred_gles") == 0)
    backend = HardwareBackend::OpenGLES;
  else if (std::strcmp(scenario, "preferred_none") == 0)
    backend = HardwareBackend::None;
  void* coreLibrary = dlopen(argv[2], RTLD_NOW | RTLD_LOCAL);
  Require(coreLibrary != nullptr, "fixture core must load");
  const auto getCoreState = reinterpret_cast<ReloadCoreState* (*)()>(
      dlsym(coreLibrary, "test_core_state"));
  Require(getCoreState != nullptr, "fixture state unavailable");
  const auto environment = reinterpret_cast<bool (*)(unsigned, void*)>(
      dlsym(coreLibrary, "test_core_environment"));
  Require(environment != nullptr, "fixture environment unavailable");
  ReloadCoreState& core = *getCoreState();
  core.hardware = testHardware || testGrowth;
  core.growGeometry = testGrowth;
  geometryGrowth = testGrowth;
  void* library = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
  if (!library)
  {
    std::fprintf(stderr, "%s\n", dlerror());
    return EXIT_FAILURE;
  }
  const auto create = reinterpret_cast<ADDON_STATUS (*)(KODI_HANDLE)>(dlsym(library, "ADDON_Create"));
  Require(create != nullptr, "wrapper must export ADDON_Create");

  AddonToKodiFuncTable_kodi_addon addonCallbacks{};
  addonCallbacks.get_addon_info = [](KODI_ADDON_BACKEND_HDL, const char*) { return strdup("test"); };
  AddonToKodiFuncTable_kodi_filesystem filesystemCallbacks{};
  filesystemCallbacks.directory_exists = [](void*, const char*) { return true; };
  filesystemCallbacks.file_exists = [](void*, const char*, bool) { return false; };
  AddonToKodiFuncTable_kodi_network networkCallbacks{};
  networkCallbacks.get_user_agent = [](void*) { return strdup("reload-test"); };
  AddonToKodiFuncTable_Addon callbacks{};
  callbacks.free_string = [](KODI_ADDON_BACKEND_HDL, char* value) { std::free(value); };
  callbacks.addon_log_msg = [](KODI_ADDON_BACKEND_HDL, int, const char*) {};
  callbacks.kodi_addon = &addonCallbacks;
  callbacks.kodi_filesystem = &filesystemCallbacks;
  callbacks.kodi_network = &networkCallbacks;

  AddonProps_Game properties{};
  properties.game_client_dll_path = argv[2];
  properties.profile_directory = "/unused-reload-test";
  AddonToKodiFuncTable_Game gameCallbacks{};
  gameCallbacks.OpenStream = OpenStream;
  gameCallbacks.CloseStream = CloseStream;
  gameCallbacks.AddStreamData = AddStreamData;
  gameCallbacks.EnableHardwareRendering = EnableHardwareRendering;
  gameCallbacks.SetGameTiming = [](KODI_HANDLE, const game_system_timing* timing)
  {
    Require(timing->fps == 70.0 && timing->sample_rate == 48000.0,
            "SET_SYSTEM_AV_INFO lost timing");
  };
  gameCallbacks.StartStream = StartStream;
  gameCallbacks.GetStreamBuffer = GetStreamBuffer;
  gameCallbacks.ReleaseStreamBuffer = [](KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle,
                                        game_stream_buffer* buffer)
  {
    Require(streams.count(handle) == 1, "released framebuffer after stream closed");
    Require(buffer->hw_framebuffer.framebuffer == 42, "released an invalid framebuffer");
  };
  KodiToAddonFuncTable_Game gameFunctions{};
  AddonInstance_Game game{&properties, &gameCallbacks, &gameFunctions};
  currentGame = &game;
  KODI_ADDON_INSTANCE_INFO info{};
  info.type = ADDON_INSTANCE_GAME;
  info.version = ADDON_INSTANCE_VERSION_GAME;
  KODI_ADDON_INSTANCE_FUNC instanceFunctions{};
  KODI_ADDON_INSTANCE_STRUCT instance{};
  instance.info = &info;
  instance.functions = &instanceFunctions;
  instance.game = &game;
  KodiToAddonFuncTable_Addon functions{};
  AddonGlobalInterface interface{};
  interface.firstKodiInstance = &instance;
  interface.toKodi = &callbacks;
  interface.toAddon = &functions;
  const ADDON_STATUS status = create(&interface);
  Require(status == ADDON_STATUS_OK || status == ADDON_STATUS_NEED_SETTINGS, "addon creation failed");

  if (testGrowth)
  {
    Require(gameFunctions.LoadGame(&game, "reload.test") == GAME_ERROR_NO_ERROR, "growth load failed");
    game_system_timing timing{};
    Require(gameFunctions.GetGameTiming(&game, &timing) == GAME_ERROR_NO_ERROR, "growth timing failed");
    for (unsigned frame = 0; frame < 5; ++frame)
    {
      contextCurrent = true;
      Require(gameFunctions.RunFrame(&game) == GAME_ERROR_NO_ERROR, "growth frame failed");
      contextCurrent = false;
      Require(core.geometryFramebuffer == 42, "maximum change invalidated cached framebuffer");
      Require(core.resets == 1 && core.destroys == 0 && hardwareOpens == 1 && hardwareStarts == 1 &&
                  hardwareCloses == 0, "maximum change reopened hardware context");
    }
    const std::vector<std::pair<unsigned, unsigned>> expectedSizes = {
        {320, 240}, {640, 480}, {640, 480}, {320, 240},
        {800, 480}, {800, 480}, {800, 600}, {800, 600}};
    Require(framebufferSizes == expectedSizes, "maximum changes requested incorrect framebuffer sizes");
    Require(hardwarePackets == 5, "frames missing after maximum changes");
    DestroyHardwareContexts();
    Require(gameFunctions.UnloadGame(&game) == GAME_ERROR_NO_ERROR, "growth unload failed");
    Require(streams.empty() && core.destroys == 1, "growth stream did not close exactly once");
    std::puts("PASS: growth, shrink, repeated growth, ignored SET_GEOMETRY maximum and cached FBO");
  }
  else if (testPreference || testHandoff)
  {
    const auto unload = [&]
    {
      DestroyHardwareContexts();
      Require(gameFunctions.UnloadGame(&game) == GAME_ERROR_NO_ERROR, "unload failed");
      Require(streams.empty(), "unload left streams open");
      negotiated = {};
      hardwareRefused = false;
    };
    const auto runFrame = [&]
    {
      game_system_timing timing{};
      Require(gameFunctions.GetGameTiming(&game, &timing) == GAME_ERROR_NO_ERROR,
              "timing failed after preference or retained load");
      contextCurrent = core.hardware;
      Require(gameFunctions.RunFrame(&game) == GAME_ERROR_NO_ERROR, "frame failed");
      contextCurrent = false;
    };
    const auto queryPreference = [&]
    {
      for (unsigned query = 0; query < 2; ++query)
      {
        retro_hw_context_type preferred = RETRO_HW_CONTEXT_NONE;
        const bool available = environment(RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER, &preferred);
        Require(available == (backend != HardwareBackend::None), "incorrect preference availability");
        const auto expected = backend == HardwareBackend::OpenGL ? RETRO_HW_CONTEXT_OPENGL_CORE :
                              backend == HardwareBackend::OpenGLES ? RETRO_HW_CONTEXT_OPENGLES3 :
                                                                    RETRO_HW_CONTEXT_NONE;
        Require(preferred == expected, "preferred API is unavailable on frontend");
        Require(negotiated.context_type == GAME_HW_CONTEXT_NONE && !hardwareRefused,
                "preference probe left an accepted context or refusal");
        Require(streams.empty(), "preference probe opened a stream");
        core.contextType = preferred;
      }
    };
    const auto queryAfterNegotiation = [&]
    {
      const auto requests = negotiations;
      retro_hw_context_type preferred = RETRO_HW_CONTEXT_NONE;
      environment(RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER, &preferred);
      Require(negotiations == requests, "late preference query changed hardware negotiation");
      Require(negotiated.depth && negotiated.stencil && negotiated.bottom_left_origin,
              "late preference query lost negotiated context flags");
    };

    if (testPreference)
    {
      const auto availableBackend = backend;
      backend = HardwareBackend::None;
      queryPreference();
      backend = availableBackend;
      queryPreference();
      Require(!environment(RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER, nullptr),
              "null preference query should be declined");
      retro_hw_render_callback unavailable{};
      unavailable.context_type = RETRO_HW_CONTEXT_VULKAN;
      Require(!environment(RETRO_ENVIRONMENT_SET_HW_RENDER, &unavailable),
              "unavailable context was accepted after probing");
      Require(!unavailable.get_current_framebuffer && !unavailable.get_proc_address,
              "refused context left hardware callbacks installed");
      core.hardware = false;
      Require(gameFunctions.LoadGame(&game, "reload.test") == GAME_ERROR_NO_ERROR,
              "software load after preference failed");
      runFrame();
      Require(videoPackets == 1 && hardwareOpens == 0, "preference changed software fallback");
      Require(!hardwareRefused && negotiated.context_type == GAME_HW_CONTEXT_NONE,
              "software fallback retained hardware negotiation");
      unload();
      queryPreference();
    }

    if (backend != HardwareBackend::None)
    {
      core.hardware = true;
      Require(gameFunctions.LoadGame(&game, "reload.test") == GAME_ERROR_NO_ERROR,
              "SET_HW_RENDER after preference failed");
      if (testPreference)
        queryAfterNegotiation();
      runFrame();
      Require(hardwarePackets == 1 && core.resets == 1, "hardware negotiation did not render");
      if (testPreference)
        queryAfterNegotiation();
      unload();
      if (testPreference)
        queryPreference();
    }
    core.hardware = false;
    const auto pixels = videoPackets;
    const auto opens = hardwareOpens;
    Require(gameFunctions.LoadGame(&game, "reload.test") == GAME_ERROR_NO_ERROR,
            "retained software load failed");
    runFrame();
    Require(videoPackets == pixels + 1 && hardwareOpens == opens,
            "retained software load inherited hardware mode");
    unload();
    std::printf("PASS: %s\n", scenario);
  }
  else if (testHardware)
  {
    for (unsigned cycle = 0; cycle < 9; ++cycle)
    {
      failOpen = cycle == 1;
      failStart = cycle == 3;
      failFramebuffer = cycle == 5;
      zeroFramebuffer = cycle == 7;
      const bool failed = failOpen || failStart || failFramebuffer || zeroFramebuffer;
      const auto resets = core.resets;
      const auto destroys = core.destroys;
      const auto starts = hardwareStarts;
      const auto closes = hardwareCloses;
      const auto queries = framebufferQueries;
      Require(gameFunctions.LoadGame(&game, "reload.test") == GAME_ERROR_NO_ERROR,
              "hardware content load failed");
      game_system_timing timing{};
      const auto result = gameFunctions.GetGameTiming(&game, &timing);
      std::printf("Hardware cycle %u: timing result=%d\n", cycle + 1, result);
      Require((result == GAME_ERROR_NO_ERROR) == !failed, "incorrect hardware startup result");
      Require(hardwareOpens == cycle + 1, "hardware stream must open once per load");
      Require(hardwareStarts == starts + (failOpen ? 0 : 1), "hardware start count differs");
      const unsigned expectedResets = failOpen || failFramebuffer || zeroFramebuffer ? 0 : 1;
      Require(core.resets == resets + expectedResets, "core reset must occur exactly once after preflight");
      if (failed)
      {
        Require(streams.empty(), "failed hardware startup left an open stream");
        Require(!core.ready, "failed hardware startup left core GPU state alive");
      }
      else
      {
        Require(core.resetFramebuffer == 42, "reset must acquire framebuffer through installed handle");
        Require(core.frames == 0, "hardware startup ran a gameplay frame");
        Require(framebufferQueries > queries, "reopen reused a stale cached framebuffer");
        const auto sizeQueries = core.preframeSizeQueries;
        Require(gameFunctions.SerializeSize(&game) == 8, "preframe serialization size unavailable");
        Require(core.preframeSizeQueries == sizeQueries + 1, "size queried before GPU initialization");
        const uint8_t state[8]{};
        const auto restores = core.restores;
        Require(gameFunctions.Deserialize(&game, state, sizeof(state)) == GAME_ERROR_NO_ERROR,
                "preframe state restoration failed");
        Require(core.restores == restores + 1 && core.frames == 0, "restoration required a gameplay frame");
        const auto packets = hardwarePackets;
        for (unsigned frame = 0; frame < 4; ++frame)
        {
          contextCurrent = true;
          Require(gameFunctions.RunFrame(&game) == GAME_ERROR_NO_ERROR, "hardware frame failed");
          contextCurrent = false;
        }
        Require(hardwarePackets == packets + 4, "hardware frames were dropped");
        Require(core.resets == resets + 1, "libretro layered an extra reset over stream startup");
      }
      DestroyHardwareContexts();
      Require(core.destroys == destroys + (failOpen ? 0 : 1), "core must release GPU state before unload");
      Require(gameFunctions.UnloadGame(&game) == GAME_ERROR_NO_ERROR, "hardware unload failed");
      Require(streams.empty(), "hardware unload must close all streams");
      Require(hardwareCloses == closes + (failOpen ? 0 : 1), "hardware stream closed more than once");
      Require(core.destroys == destroys + (failOpen ? 0 : 1), "hardware context destroyed more than once");
      Require(core.destroysAfterUnload == 0, "hardware context destroyed after core unload");
    }
    std::puts("PASS: hardware reset, framebuffer, restoration, DAR, rotation, "
              "failures, and reload");
  }
  else
  {
    for (unsigned cycle = 0; cycle < 6; ++cycle)
    {
      const bool standalone = (cycle % 3 == 2);
      std::printf("Cycle %u (%s)\n", cycle + 1, standalone ? "standalone" : "content");
      const GAME_ERROR loaded = standalone ? gameFunctions.LoadStandalone(&game)
                                           : gameFunctions.LoadGame(&game, "reload.test");
      Require(loaded == GAME_ERROR_NO_ERROR, "load failed on retained addon instance");
      game_system_timing timing{};
      const GAME_ERROR timingResult = gameFunctions.GetGameTiming(&game, &timing);
      std::printf("  timing: %g FPS, %g Hz; result=%d\n", timing.fps, timing.sample_rate, timingResult);
      Require(timingResult == GAME_ERROR_NO_ERROR, "GetGameTiming failed after load");
      Require(timing.fps == 70.0 && timing.sample_rate == 48000.0, "incorrect timing");
      Require(gameFunctions.RunFrame(&game) == GAME_ERROR_NO_ERROR, "running frame failed");
      Require(videoPackets == cycle + 1, "video did not resume after load");
      Require(audioPackets == cycle + 1, "audio did not resume after load");
      Require(streams.size() == 2, "expected one audio and one video stream");
      Require(gameFunctions.UnloadGame(&game) == GAME_ERROR_NO_ERROR, "unloading failed");
      Require(streams.empty(), "unload must close all streams");
    }
  }
  functions.destroy(interface.addonBase);
  dlclose(library);
  dlclose(coreLibrary);
  if (std::strcmp(scenario, "software") == 0)
    std::puts("PASS: timing, video, and audio survive six loads on one addon instance");
}
