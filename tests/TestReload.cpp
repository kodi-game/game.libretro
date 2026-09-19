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

namespace
{
std::map<KODI_GAME_STREAM_HANDLE, GAME_STREAM_TYPE> streams;
unsigned videoPackets = 0;
unsigned audioPackets = 0;

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
  auto* handle = new int;
  streams.emplace(handle, properties->type);
  return handle;
}

void CloseStream(KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle)
{
  Require(streams.erase(handle) == 1, "closing an unknown stream");
  delete static_cast<int*>(handle);
}

void AddStreamData(KODI_HANDLE, KODI_GAME_STREAM_HANDLE handle, const game_stream_packet* packet)
{
  Require(streams.count(handle) == 1, "packet sent to a closed stream");
  Require(streams.at(handle) == packet->type, "packet type differs from stream type");
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
  else
    Require(false, "unexpected stream type");
}
}

int main(int argc, char** argv)
{
  Require(argc == 3, "usage: test_reload <wrapper library> <fixture core library>");
  void* coreLibrary = dlopen(argv[2], RTLD_NOW | RTLD_LOCAL);
  Require(coreLibrary != nullptr, "fixture core must load");
  const auto getCoreState = reinterpret_cast<ReloadCoreState* (*)()>(
      dlsym(coreLibrary, "test_core_state"));
  Require(getCoreState != nullptr, "fixture state unavailable");
  ReloadCoreState& core = *getCoreState();
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
  filesystemCallbacks.stat_file = [](void*, const char*, STAT_STRUCTURE*) { return false; };
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
  KodiToAddonFuncTable_Game gameFunctions{};
  AddonInstance_Game game{&properties, &gameCallbacks, &gameFunctions};
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
    Require(core.loadAttempts == cycle + 1, "core did not load on retained instance");
    Require(core.frames == cycle + 1, "frame did not reach the retained core");
    // Piers initializes media streams only when the addon instance is created.
    if (cycle == 0)
    {
      Require(videoPackets == 1, "video missing on initial load");
      Require(audioPackets == 1, "audio missing on initial load");
      Require(streams.size() == 2, "expected one audio and one video stream");
    }
    Require(gameFunctions.UnloadGame(&game) == GAME_ERROR_NO_ERROR, "unloading failed");
    Require(streams.empty(), "unload must close all streams");
  }
  functions.destroy(interface.addonBase);
  dlclose(library);
  dlclose(coreLibrary);
  std::puts("PASS: timing and core frames survive six loads on one addon instance");
}
