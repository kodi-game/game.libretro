/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "memory/CheevosMemory.h"
#include "memory/LibretroMemory.h"

#include <rcheevos/rc_consoles.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

using namespace LIBRETRO;

namespace
{
void Require(bool condition, const char* message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(EXIT_FAILURE);
  }
}

uint8_t systemRam[16]{1, 2, 3, 4};
uint8_t saveRam[8]{5, 6, 7, 8};
bool available = true;
unsigned calls = 0;
unsigned mappingMessages = 0;

void* GetData(unsigned type)
{
  ++calls;
  if (!available)
    return nullptr;
  if (type == RETRO_MEMORY_SYSTEM_RAM)
    return systemRam;
  if (type == RETRO_MEMORY_SAVE_RAM)
    return saveRam;
  return nullptr;
}
size_t GetSize(unsigned type)
{
  ++calls;
  if (!available)
    return 0;
  if (type == RETRO_MEMORY_SYSTEM_RAM)
    return sizeof(systemRam);
  if (type == RETRO_MEMORY_SAVE_RAM)
    return sizeof(saveRam);
  return 0;
}
void RC_CCONV OnMappingMessage(const char*) { ++mappingMessages; }

bool Set(CLibretroMemory& memory, std::initializer_list<retro_memory_descriptor> descriptors)
{
  return memory.SetMemoryMap({descriptors.begin(), static_cast<unsigned>(descriptors.size())});
}

void TestFallbackAndRefresh()
{
  CLibretroMemory memory;
  memory.Initialize(GetData, GetSize);
  memory.BeginContent();
  CCheevosMemory view(memory);
  uint8_t buffer[4]{};
  Require(view.Read(RC_CONSOLE_HUBS, 0, buffer, 4) == 4 && buffer[0] == 1 && buffer[3] == 4,
          "no map falls back to standard system RAM");
  Require(view.Read(RC_CONSOLE_HUBS, sizeof(systemRam), buffer, 4) == 4 && buffer[0] == 5,
          "no map falls back to standard save RAM");
  Require(view.Read(RC_CONSOLE_HUBS, sizeof(systemRam) - 1, buffer, 2) == 2 && buffer[1] == 5,
          "read across system/save buffer boundary");
  const auto firstCalls = calls;
  for (unsigned i = 0; i < 100; ++i)
    Require(view.Read(RC_CONSOLE_HUBS, 0, buffer, 1) == 1, "cached read");
  Require(calls == firstCalls, "same generation and console do not rebuild");

  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0x10000, buffer, 1) == 1 && buffer[0] == 5,
          "console ID changes RA's save RAM logical address");
  Require(calls > firstCalls, "console ID change rebuilds without a map change");
  const auto consoleCalls = calls;
  Require(memory.SetMemoryMap({nullptr, 0}), "empty replacement accepted");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 1 && calls > consoleCalls,
          "generation change rebuilds fallback too");

  view.Reset();
  available = false;
  const auto fallbackGeneration = memory.Generation();
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 0,
          "reset drops old pointers even when generation did not change");
  const auto failedCalls = calls;
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 0 && calls > failedCalls,
          "failed init is not marked initialized");
  available = true;
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 1 && buffer[0] == 1,
          "later memory availability retries without generation change");
  Require(memory.Generation() == fallbackGeneration, "flat fallback retry keeps the same memory generation");
  memory.EndContent();
  const auto endedCalls = calls;
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 0 && calls == endedCalls,
          "ended content invalidates fallback without calling core");
  memory.BeginContent();
  Require(view.Read(RC_CONSOLE_HUBS, 0, buffer, 1) == 1, "retained instance fallback resumes");
  view.Reset();
  memory.Deinitialize();
  Require(view.Read(RC_CONSOLE_HUBS, 0, buffer, 1) == 0, "no view survives core teardown");
  Require(view.Read(RC_CONSOLE_HUBS, 0, nullptr, 1) == 0 &&
              view.Read(RC_CONSOLE_HUBS, 0, buffer, 0) == 0, "empty read handling");
}

void TestUnavailableDescriptors()
{
  CLibretroMemory memory;
  memory.Initialize(GetData, GetSize);
  memory.BeginContent();
  CCheevosMemory view(memory);
  const retro_memory_descriptor informational{0, nullptr, 0, 0, 0xffffff, 0, 0, nullptr};
  Require(Set(memory, {informational}), "install inaccessible descriptor map");
  const auto generation = memory.Generation();
  const auto initialCalls = calls;
  auto messages = mappingMessages;
  uint8_t value = 0;
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, &value, 1) == 0 && mappingMessages > messages,
          "first descriptor failure attempts mapping");
  messages = mappingMessages;
  for (unsigned i = 0; i < 100; ++i)
    Require(view.Read(RC_CONSOLE_MEGA_DRIVE, i, &value, 1) == 0, "unavailable descriptor read");
  Require(mappingMessages == messages, "same failed key must not repeat mapping or diagnostics");
  Require(calls == initialCalls, "inaccessible descriptor map must not use available flat RAM");
  Require(!memory.SetMemoryMap({nullptr, 1}), "reject replacement of unavailable map");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, &value, 1) == 0 && mappingMessages == messages,
          "rejected replacement preserves cached unavailability");

  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 0 && mappingMessages > messages,
          "different console retries the same unavailable memory generation");
  messages = mappingMessages;
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 0 && mappingMessages == messages,
          "different console failure is cached under its own key");
  view.Reset();
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 0 && mappingMessages > messages,
          "Reset clears cached unavailability");
  Require(memory.Generation() == generation, "console change and Reset keep the same source generation");
  messages = mappingMessages;
  Require(Set(memory, {informational}), "identical replacement advances memory generation");
  Require(memory.Generation() == generation + 1, "accepted replacement changes the failed key");
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 0 && mappingMessages > messages,
          "new generation retries even an identical inaccessible map");
  Require(calls == initialCalls, "descriptor failure never calls flat fallback");

  // Keep the console unchanged so only the new generation can invalidate failure.
  std::vector<uint8_t> ram(0x800, 0x42);
  messages = mappingMessages;
  Require(Set(memory, {{0, ram.data(), 0, 0, 0, 0, ram.size(), nullptr}}), "install accessible NES RAM");
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 1 && value == 0x42 &&
              mappingMessages > messages, "valid replacement rebuilds and succeeds after cached failure");
  messages = mappingMessages;
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 1 && mappingMessages == messages,
          "successful replacement stays cached");

  Require(Set(memory, {informational}), "reinstall unavailable map");
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 0, "cache descriptor failure again");
  Require(memory.SetMemoryMap({nullptr, 0}), "empty map removes unavailable descriptors");
  Require(view.Read(RC_CONSOLE_NINTENDO, 0, &value, 1) == 1 && value == systemRam[0] &&
              calls > initialCalls, "empty replacement restores standard-memory fallback");
}

void TestDescriptorMaps()
{
  CLibretroMemory memory;
  memory.Initialize(GetData, GetSize);
  memory.BeginContent();
  CCheevosMemory view(memory);
  // Mega Drive layout/addresses follow upstream test_rc_libretro.c.
  std::unique_ptr<uint8_t[]> oldRam(new uint8_t[0x10000]{});
  std::vector<uint8_t> newRam(0x10000, 0x32);
  std::vector<uint8_t> otherRam(0x8000, 0x54);
  std::vector<uint8_t> sram(0x10000, 0x76);
  oldRam[0] = 0x10;
  Require(Set(memory, {{RETRO_MEMDESC_SYSTEM_RAM, oldRam.get(), 0, 0xff0000, 0, 0, 0x10000, "RAM"},
                       {RETRO_MEMDESC_SAVE_RAM, sram.data(), 0, 0, 0, 0, 0x10000, "SRAM"}}),
          "descriptor map accepted");
  uint8_t buffer[4]{};
  const auto initialCalls = calls;
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 1 && buffer[0] == 0x10,
          "RA reads normalized descriptor map");
  Require(calls == initialCalls, "descriptor map takes precedence over flat callbacks");
  const auto messages = mappingMessages;
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0x10000, buffer, 1) == 1 && buffer[0] == 0x76,
          "RA maps console save RAM");
  Require(mappingMessages == messages, "descriptor view not rebuilt for repeated reads");

  Require(Set(memory, {{RETRO_MEMDESC_SYSTEM_RAM, newRam.data(), 0, 0xff0000, 0, 0, 0x10000, "RAM"},
                       {RETRO_MEMDESC_SAVE_RAM, sram.data(), 0, 0, 0, 0, 0x10000, "SRAM"}}),
          "new backing pointer accepted");
  oldRam.reset(); // A stale derived view is now detectable by ASan as well as value.
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 1 && buffer[0] == 0x32,
          "generation change replaces old backing pointer");
  Require(mappingMessages > messages, "generation change actually rebuilt descriptor view");
  const auto beforeRejected = mappingMessages;
  Require(!memory.SetMemoryMap({nullptr, 1}), "invalid map rejected");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 1 && buffer[0] == 0x32 &&
              mappingMessages == beforeRejected, "rejection keeps valid RA view");

  Require(Set(memory, {{0, newRam.data(), 0, 0xff0000, 0, 0, 0x8000, nullptr},
                       {0, otherRam.data(), 0, 0xff8000, 0, 0, 0x8000, nullptr},
                       {0, sram.data(), 0, 0, 0, 0, 0x10000, nullptr}}), "split topology");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0x7ffe, buffer, 4) == 4 &&
              buffer[0] == 0x32 && buffer[1] == 0x32 && buffer[2] == 0x54 && buffer[3] == 0x54,
          "topology replacement changes logical reads across boundary");

  // With raw disconnect=0, normalized disconnect=0xc000 supplies four mirrors.
  Require(Set(memory, {{0, otherRam.data(), 4, 0xff0000, 0xff0000, 0, 0x4000, nullptr}}),
          "mirrored descriptor accepted");
  otherRam[6] = 0x98;
  Require(memory.Descriptors()[0].disconnect == 0xc000, "normalization adds mirror bits");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0xc002, buffer, 1) == 1 && buffer[0] == 0x98,
          "RA received normalized mirrors and buffer offset");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0x10000, buffer, 1) == 0,
          "missing region is not filled with unrelated flat memory");

  Require(Set(memory, {{0, nullptr, 0, 0, 0xffffff, 0, 0, nullptr}}), "informational map accepted");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 0, "no accessible descriptor is init failure");
  Require(Set(memory, {{0, newRam.data(), 0, 0xff0000, 0, 0, 0x10000, nullptr}}), "retry map");
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 1 && buffer[0] == 0x32,
          "valid map recovers after descriptor init failure");
  view.Reset(); // Owner tears derived memory down before freeing content.
  memory.EndContent();
  newRam.clear();
  newRam.shrink_to_fit();
  available = false;
  memory.BeginContent();
  Require(view.Read(RC_CONSOLE_MEGA_DRIVE, 0, buffer, 1) == 0,
          "new content without replacement cannot reuse previous descriptor pointers");
  available = true;
}

uint8_t nestedRam[4]{0xa1, 0xa2, 0xa3, 0xa4};
CCheevosMemory* nestedView = nullptr;
void* NestedData(unsigned type) { return type == RETRO_MEMORY_SYSTEM_RAM ? nestedRam : nullptr; }
size_t NestedSize(unsigned type) { return type == RETRO_MEMORY_SYSTEM_RAM ? sizeof(nestedRam) : 0; }
void* OuterData(unsigned type)
{
  if (type == RETRO_MEMORY_SYSTEM_RAM)
  {
    uint8_t value = 0;
    Require(nestedView->Read(RC_CONSOLE_HUBS, 0, &value, 1) == 1 && value == 0xa1,
            "nested initialization selects its own core");
  }
  return GetData(type);
}

void TestCallbackScope()
{
  for (unsigned cycle = 0; cycle < 20; ++cycle)
  {
    CLibretroMemory inner;
    inner.Initialize(NestedData, NestedSize);
    inner.BeginContent();
    CCheevosMemory innerView(inner);
    nestedView = &innerView;
    CLibretroMemory outer;
    outer.Initialize(OuterData, GetSize);
    outer.BeginContent();
    CCheevosMemory outerView(outer);
    uint8_t value = 0;
    Require(outerView.Read(RC_CONSOLE_HUBS, sizeof(systemRam), &value, 1) == 1 && value == 5,
            "nested bridge restores the outer instance for the next callback");
    outerView.Reset();
    available = false;
    Require(outerView.Read(RC_CONSOLE_HUBS, 0, &value, 1) == 0, "failed scoped initialization");
    available = true;
    Require(outerView.Read(RC_CONSOLE_HUBS, 0, &value, 1) == 1 && value == 1,
            "retry after failure has correct callback instance");
    nestedView = nullptr;
  }
}
} // namespace

int main()
{
  rc_libretro_init_verbose_message_callback(OnMappingMessage);
  TestFallbackAndRefresh();
  TestUnavailableDescriptors();
  TestDescriptorMaps();
  TestCallbackScope();
  rc_libretro_init_verbose_message_callback(nullptr);
  std::puts("PASS: pinned rcheevos fallback, refresh, replacement, failure/retry, teardown and callback scope");
}
