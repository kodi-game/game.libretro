/*
 *  Copyright (C) 2014-2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct retro_memory_map;

namespace LIBRETRO
{
struct MemoryDescriptor
{
  uint64_t flags{0};
  // Non-owning, live core memory. Normalization never dereferences this pointer.
  uint8_t* data{nullptr};
  size_t offset{0};
  size_t start{0};
  size_t select{0};
  size_t disconnect{0};
  size_t length{0};
  std::string addressSpace;
};

// An owned, normalized value. Only the emulated RAM itself belongs to the core.
class CMemoryMap
{
public:
  // Rejection leaves the previous value intact. An empty map clears it.
  bool Initialize(const retro_memory_map& map);
  void Clear() { m_descriptors.clear(); }
  const std::vector<MemoryDescriptor>& Descriptors() const { return m_descriptors; }

private:
  static bool Normalize(std::vector<MemoryDescriptor>& descriptors);

  std::vector<MemoryDescriptor> m_descriptors;
};
} // namespace LIBRETRO
