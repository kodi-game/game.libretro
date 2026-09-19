/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "memory/LibretroMemory.h"

#include <libretro.h>

#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
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

bool Set(CLibretroMemory& memory, std::initializer_list<retro_memory_descriptor> descriptors)
{
  return memory.SetMemoryMap({descriptors.begin(), static_cast<unsigned>(descriptors.size())});
}

bool Equal(const MemoryDescriptor& left, const MemoryDescriptor& right)
{
  return left.flags == right.flags && left.data == right.data && left.offset == right.offset &&
         left.start == right.start && left.select == right.select &&
         left.disconnect == right.disconnect && left.length == right.length &&
         left.addressSpace == right.addressSpace;
}

void TestReplacement()
{
  CLibretroMemory memory;
  Require(memory.Generation() == 0 && !memory.HasMemoryMap(), "initial state");
  uint8_t ram[32]{};
  char first[] = "RAM";
  std::string second = "VRAM";
  Require(Set(memory, {{RETRO_MEMDESC_SYSTEM_RAM, ram, 3, 0, 0, 0, 16, first},
                       {RETRO_MEMDESC_VIDEO_RAM, ram, 0, 16, 0, 0, 16, second.c_str()}}),
          "accept two descriptors");
  first[0] = 'X';
  second.assign(256, 'X');
  Require(memory.Descriptors()[0].addressSpace == "RAM" &&
              memory.Descriptors()[1].addressSpace == "VRAM", "independently owned names");
  Require(memory.Descriptors()[0].data == ram && memory.Descriptors()[0].offset == 3,
          "RAM pointer is borrowed, not copied or offset");
  Require(memory.Generation() == 1, "accepted generation");
  const auto original = memory.Descriptors();
  const auto generation = memory.Generation();
  const auto reject = [&](std::initializer_list<retro_memory_descriptor> descriptors)
  {
    Require(!Set(memory, descriptors), "reject invalid candidate");
    Require(memory.Generation() == generation && memory.Descriptors().size() == original.size(),
            "rejection preserves generation and size");
    for (size_t i = 0; i < original.size(); ++i)
      Require(Equal(memory.Descriptors()[i], original[i]), "rejection preserves every field");
  };
  // The first descriptor normalizes successfully before the second fails.
  reject({{0, ram, 0, 0, 0, 0, 8, "NEW"}, {0, ram, 0, 1, 2, 0, 8, nullptr}});
  reject({{0, ram, 0, 0, 0, 0, 3, nullptr}});
  reject({{0, ram, 0, 0, 0, 0, 0, nullptr}});
  const size_t maximum = std::numeric_limits<size_t>::max();
  reject({{0, ram, 0, maximum, 0, 0, 2, nullptr}});
  reject({{0, ram, maximum, 0, 0, 0, 2, nullptr}});
  // A high address-space bit plus only a low select bit infers SIZE_MAX + 1.
  reject({{0, nullptr, 0, 0, 1, 0, 0, nullptr},
          {0, nullptr, 0, 0, maximum, 0, 0, nullptr}});
  Require(!memory.SetMemoryMap({nullptr, 1}), "null descriptor array rejected");
  Require(memory.Generation() == generation && Equal(memory.Descriptors()[0], original[0]),
          "null descriptor array preserves state");

  const retro_memory_descriptor replacement{0, ram, 0, 0, 0, 0, 32, nullptr};
  Require(Set(memory, {replacement}) && memory.Descriptors().size() == 1,
          "replacement must not append");
  Require(memory.Descriptors()[0].addressSpace.empty(), "null name is empty");
  const auto replacedGeneration = memory.Generation();
  Require(Set(memory, {replacement}) && memory.Generation() == replacedGeneration + 1,
          "identical replacement advances generation");
  for (unsigned i = 0; i < 1000; ++i)
  {
    std::string name = "SPACE" + std::to_string(i);
    retro_memory_descriptor descriptor = replacement;
    descriptor.addrspace = name.c_str();
    Require(Set(memory, {descriptor}), "repeated replacement");
    Require(memory.Descriptors().size() == 1 && memory.Descriptors()[0].addressSpace == name,
            "no descriptor accumulation or name leakage");
  }
  const auto beforeEmpty = memory.Generation();
  Require(memory.SetMemoryMap({nullptr, 0}), "empty map accepted");
  Require(!memory.HasMemoryMap() && memory.Generation() == beforeEmpty + 1,
          "empty map explicitly clears and advances");
  Require(memory.SetMemoryMap({&replacement, 0}), "zero count ignores nonnull array");
  Require(memory.Generation() == beforeEmpty + 2, "repeated empty replacement advances");
}

void CheckNormalized(const MemoryDescriptor& descriptor, size_t select, size_t disconnect, size_t length)
{
  Require(descriptor.select == select, "normalized select matches RetroArch");
  Require(descriptor.disconnect == disconnect, "normalized disconnect matches RetroArch");
  Require(descriptor.length == length, "normalized length matches RetroArch");
}

void TestNormalization()
{
  CLibretroMemory memory;
  Require(Set(memory, {{0, nullptr, 0, 0x2000, 0, 0, 0x2000, nullptr}}), "implied select");
  CheckNormalized(memory.Descriptors()[0], 0x2000, 0, 0x2000);
  Require(Set(memory, {{0, nullptr, 0, 0, 0xc000, 0x2000, 0, nullptr}}), "implied length");
  CheckNormalized(memory.Descriptors()[0], 0xc000, 0x2000, 0x2000);

  // Recorded from current upstream mmap_preprocess_descriptors(), not computed
  // with the implementation under test. Each disconnect straddles length's bits.
  const size_t vectors[][4] = {
      {0x8000, 0x110, 0x100, 0x7d10},
      {0x80, 0x12, 0x10, 0x52},
      {0x8000, 0x210, 0x180, 0x7a10},
  };
  for (const auto& v : vectors)
  {
    Require(Set(memory, {{0, nullptr, 0, 0, v[0], v[1], v[2], nullptr}}), "disconnect regression");
    CheckNormalized(memory.Descriptors()[0], v[0], v[3], v[2]);
  }
  // Explicit select permits non-power-of-two and bankswitched lengths.
  Require(Set(memory, {{0, nullptr, 0, 0, 0xff, 0, 0x180, nullptr}}), "bankswitched length accepted");
  CheckNormalized(memory.Descriptors()[0], 0xff, 0, 0x180);
  const size_t maximum = std::numeric_limits<size_t>::max();
  Require(Set(memory, {{0, nullptr, 0, maximum, maximum, maximum, 0, nullptr}}),
          "full-width select/disconnect and legal null backing pointer");
  CheckNormalized(memory.Descriptors()[0], maximum, maximum, 1);
  Require(Set(memory, {{0, nullptr, 0, maximum, 0, 0, 1, nullptr}}), "last address does not overflow");
  CheckNormalized(memory.Descriptors()[0], maximum, 0, 1);
}

void TestSnesExamples()
{
  CLibretroMemory memory;
  // The 14 descriptors in libretro.h, including informational descriptors with
  // no backing pointer. See MemoryModel.md for recorded upstream oracle results.
  Require(Set(memory, {
      {0, nullptr, 0, 0x7e0000, 0, 0, 0x20000, "WRAM"},
      {0, nullptr, 0, 0, 0, 0, 0x10000, "SPC700"},
      {0, nullptr, 0, 0, 0xc0e000, 0, 0x2000, "WRAM"},
      {0, nullptr, 0, 0x800000, 0xc0e000, 0, 0x2000, "WRAM"},
      {0, nullptr, 0, 0, 0x40e000, ~size_t(0x1fff), 0, "WRAM"},
      {RETRO_MEMDESC_CONST, nullptr, 0, 0x008000, 0x408000, 0x8000, 0x80000, "LoROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0, 0x400000, 0x400000, 0x8000, 0x80000, "LoROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0, 0x400000, 0x400000, 0, 0x400000, "HiROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0x8000, 0x008000, 0x408000, 0, 0x400000, "HiROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0, 0xc00000, 0xc00000, 0, 0x400000, "ExHiROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0x400000, 0x400000, 0xc00000, 0, 0x400000, "ExHiROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0x8000, 0x808000, 0xc08000, 0, 0x400000, "ExHiROM"},
      {RETRO_MEMDESC_CONST, nullptr, 0x408000, 0x008000, 0xc08000, 0, 0x400000, "ExHiROM"},
      {0, nullptr, 0, 0, 0xffffff, 0, 0, nullptr}}), "SNES examples accepted");
  const size_t expected[][3] = {
      {0xfe0000, 0, 0x20000}, {0xff0000, 0, 0x10000},
      {0xc0e000, 0x3f0000, 0x2000}, {0xc0e000, 0x3f0000, 0x2000},
      {0x40e000, ~size_t(0x1fff), 0x2000},
      {0x408000, 0xb08000, 0x80000}, {0x400000, 0xb08000, 0x80000},
      {0x400000, 0x800000, 0x400000}, {0x408000, 0x800000, 0x400000},
      {0xc00000, 0, 0x400000}, {0xc00000, 0, 0x400000},
      {0xc08000, 0, 0x400000}, {0xc08000, 0, 0x400000}, {0xffffff, 0, 1}};
  for (size_t i = 0; i < memory.Descriptors().size(); ++i)
    CheckNormalized(memory.Descriptors()[i], expected[i][0], expected[i][1], expected[i][2]);
}

uint8_t flatRam[16]{};
unsigned flatCalls = 0;
void* FlatData(unsigned type)
{
  ++flatCalls;
  return type == RETRO_MEMORY_SYSTEM_RAM ? flatRam : nullptr;
}
size_t FlatSize(unsigned type)
{
  ++flatCalls;
  return type == RETRO_MEMORY_SYSTEM_RAM ? sizeof(flatRam) : 0;
}

void TestLifecycle()
{
  CLibretroMemory memory;
  uint8_t* data = flatRam;
  size_t size = 123;
  Require(!memory.GetMemory(RETRO_MEMORY_SYSTEM_RAM, data, size) && !data && !size,
          "uninitialized flat access clears outputs");
  memory.Initialize(FlatData, FlatSize);
  const auto initialGeneration = memory.Generation();
  const retro_memory_descriptor initMap{0, flatRam, 0, 0, 0, 0, sizeof(flatRam), "INIT"};
  Require(Set(memory, {initMap}), "initialization map");
  for (unsigned load = 0; load < 3; ++load)
  {
    const auto generation = memory.Generation();
    memory.BeginContent();
    Require(memory.Generation() == generation + 1 && memory.HasMemoryMap(), "load retains init map");
    Require(memory.GetMemory(RETRO_MEMORY_SYSTEM_RAM, data, size) && data == flatRam && size == 16,
            "central flat access");
    Require(!memory.GetMemory(RETRO_MEMORY_SAVE_RAM, data, size) && !data && !size,
            "unavailable flat type");
    // An invalid load-time replacement must not change the init map's lifetime.
    Require(!memory.SetMemoryMap({nullptr, 1}), "invalid load map");
    memory.EndContent(); // Same cleanup on success/unload and load failure.
    Require(memory.Generation() == generation + 2 && memory.HasMemoryMap(), "unload retains init map");
    const auto calls = flatCalls;
    Require(!memory.GetMemory(RETRO_MEMORY_SYSTEM_RAM, data, size) && !data && !size,
            "flat buffers unavailable between contents");
    Require(flatCalls == calls, "no core memory calls outside content lifetime");
  }
  memory.BeginContent();
  Require(Set(memory, {{0, flatRam, 0, 0, 0, 0, 8, "CONTENT"}}), "content replacement");
  const auto contentGeneration = memory.Generation();
  memory.EndContent();
  Require(!memory.HasMemoryMap() && memory.Generation() == contentGeneration + 1,
          "content map invalidated without resurrecting superseded init pointers");
  memory.BeginContent();
  Require(!memory.HasMemoryMap(), "new content without replacement has no stale map");
  Require(Set(memory, {initMap}), "failed load can publish a map");
  memory.EndContent();
  memory.BeginContent();
  Require(!memory.HasMemoryMap(), "failed load map cannot contaminate retry");
  memory.EndContent();
  Require(memory.Generation() > initialGeneration, "generation increases across content lifetimes");
  memory.Deinitialize();
  Require(!memory.HasMemoryMap(), "deinit clears core map");
  memory.Initialize(FlatData, FlatSize);
  Require(Set(memory, {initMap}), "second core init map");
  memory.Deinitialize();
  Require(!memory.HasMemoryMap() && !memory.GetMemory(RETRO_MEMORY_SYSTEM_RAM, data, size),
          "core deinit clears descriptors and callable memory functions");
  CLibretroMemory nextInstance;
  Require(!nextInstance.HasMemoryMap() && nextInstance.Generation() == 0, "no instance state leakage");
}
} // namespace

int main()
{
  TestReplacement();
  TestNormalization();
  TestSnesExamples();
  TestLifecycle();
  std::puts("PASS: memory ownership, replacement, normalization, generation, lifecycle and flat access");
}
