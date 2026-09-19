/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <rcheevos/rc_libretro.h>

#include <cstdint>

namespace LIBRETRO
{
class CLibretroMemory;

// Game-thread-only RA view of the core's memory. No rc_client, network or UI state.
class CCheevosMemory
{
public:
  explicit CCheevosMemory(const CLibretroMemory& memory) : m_memory(memory) {}
  ~CCheevosMemory() { Reset(); }
  CCheevosMemory(const CCheevosMemory&) = delete;
  CCheevosMemory& operator=(const CCheevosMemory&) = delete;

  uint32_t Read(uint32_t consoleId, uint32_t address, uint8_t* buffer, uint32_t size);
  void Reset();

private:
  enum class InitializationState
  {
    UNINITIALIZED,
    READY,
    UNAVAILABLE,
  };

  bool Refresh(uint32_t consoleId);

  const CLibretroMemory& m_memory;
  rc_libretro_memory_regions_t m_regions{};
  uint64_t m_generation{0};
  uint32_t m_consoleId{0};
  InitializationState m_state{InitializationState::UNINITIALIZED};
};
} // namespace LIBRETRO
