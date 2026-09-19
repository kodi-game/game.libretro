/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "MemoryMap.h"

namespace LIBRETRO
{
// Owned by CGameLibRetro. All operations, including core callbacks, run on the
// game thread. Readers must rederive borrowed memory views when Generation changes.
class CLibretroMemory
{
public:
  using GetMemoryData = void* (*)(unsigned);
  using GetMemorySize = size_t (*)(unsigned);

  // Install the loaded core's functions before retro_set_environment/retro_init.
  void Initialize(GetMemoryData getData, GetMemorySize getSize);
  void Deinitialize();
  void BeginContent();
  void EndContent();

  bool SetMemoryMap(const retro_memory_map& map);

  // Versions all borrowed memory views: descriptor topology/pointers, flat
  // buffers, and content lifetime. Discard cached views when this changes.
  // Equality is meaningful only within this owner's lifetime; zero is not a
  // validity sentinel. uint64_t wraparound is not expected in that lifetime.
  uint64_t Generation() const { return m_generation; }
  bool HasMemoryMap() const { return !Descriptors().empty(); }
  const std::vector<MemoryDescriptor>& Descriptors() const { return m_map.Descriptors(); }

  // False means unavailable. Empty outputs and the Kodi ABI's success result
  // are handled separately by CGameLibRetro::GetMemory().
  bool GetMemory(unsigned type, uint8_t*& data, size_t& size) const;

private:
  CMemoryMap m_map;
  uint64_t m_generation{0};
  GetMemoryData m_getData{nullptr};
  GetMemorySize m_getSize{nullptr};
  bool m_contentActive{false};
  bool m_contentMap{false};
};
} // namespace LIBRETRO
