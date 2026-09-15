/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cstdint>

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
};
