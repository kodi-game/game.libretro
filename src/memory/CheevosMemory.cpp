/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "CheevosMemory.h"

#include "LibretroMemory.h"

#include <vector>

using namespace LIBRETRO;

namespace
{
// rc_libretro_memory_init has no callback userdata. It consumes both the map
// and callback synchronously (v12.3.0 and current upstream). Bound this bridge
// to that call, restoring an outer call if initialization is ever nested.
thread_local const CLibretroMemory* currentMemory = nullptr;

class CMemoryScope
{
public:
  explicit CMemoryScope(const CLibretroMemory& memory) : m_previous(currentMemory)
  {
    currentMemory = &memory;
  }
  ~CMemoryScope() { currentMemory = m_previous; }

private:
  const CLibretroMemory* const m_previous;
};

void RC_CCONV GetCoreMemory(uint32_t type, rc_libretro_core_memory_info_t* info)
{
  if (!info)
    return;
  info->data = nullptr;
  info->size = 0;
  if (currentMemory)
    currentMemory->GetMemory(type, info->data, info->size);
}
} // namespace

void CCheevosMemory::Reset()
{
  // Also clears any partial/null regions left by a failed initialization.
  rc_libretro_memory_destroy(&m_regions);
  m_state = InitializationState::UNINITIALIZED;
  m_generation = 0;
  m_consoleId = 0;
}

bool CCheevosMemory::Refresh(uint32_t consoleId)
{
  const uint64_t generation = m_memory.Generation();
  if (m_state != InitializationState::UNINITIALIZED &&
      m_generation == generation && m_consoleId == consoleId)
    return m_state == InitializationState::READY;

  Reset();

  std::vector<retro_memory_descriptor> descriptors;
  descriptors.reserve(m_memory.Descriptors().size());
  for (const auto& source : m_memory.Descriptors())
  {
    retro_memory_descriptor descriptor{};
    descriptor.flags = source.flags;
    descriptor.ptr = source.data;
    descriptor.offset = source.offset;
    descriptor.start = source.start;
    descriptor.select = source.select;
    descriptor.disconnect = source.disconnect;
    descriptor.len = source.length;
    descriptor.addrspace = source.addressSpace.c_str();
    descriptors.emplace_back(descriptor);
  }
  const retro_memory_map map{descriptors.data(), static_cast<unsigned>(descriptors.size())};

  int result;
  {
    CMemoryScope scope(m_memory);
    // Only derived RAM pointers/sizes survive init; descriptor and string
    // metadata are not retained by rcheevos. The source stays stable on this thread.
    result = rc_libretro_memory_init(&m_regions, descriptors.empty() ? nullptr : &map,
                                    GetCoreMemory, consoleId);
  }
  if (!result)
  {
    Reset();
    // Standard RAM may appear without a new memory generation. A descriptor
    // map's pointers/topology, however, stay fixed until the core replaces it.
    if (descriptors.empty())
      return false;
  }

  m_generation = generation;
  m_consoleId = consoleId;
  m_state = result ? InitializationState::READY : InitializationState::UNAVAILABLE;
  return m_state == InitializationState::READY;
}

uint32_t CCheevosMemory::Read(uint32_t consoleId, uint32_t address, uint8_t* buffer, uint32_t size)
{
  if (!buffer || size == 0 || !Refresh(consoleId))
    return 0;
  return rc_libretro_memory_read(&m_regions, address, buffer, size);
}
