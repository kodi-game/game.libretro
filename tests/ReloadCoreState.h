/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cstdint>
#include <libretro.h>

struct ReloadCoreState
{
  bool hardware{};
  bool ready{};
  unsigned resets{};
  unsigned destroys{};
  unsigned destroysAfterUnload{};
  unsigned frames{};
  unsigned preframeSizeQueries{};
  unsigned restores{};
  uintptr_t resetFramebuffer{};
  retro_hw_context_type contextType{RETRO_HW_CONTEXT_OPENGL};
  bool growGeometry{};
  uintptr_t geometryFramebuffer{};
  bool memoryLoad{};
  bool probeBeforeLoad{};
  bool softwareAfterFailure{};
  unsigned failedLoads{};
  unsigned loadAttempts{};
  unsigned memoryAttempts{};
  bool emitFailureFrame{};
  unsigned failedCallbacks{};
};
