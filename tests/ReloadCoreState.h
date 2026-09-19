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
  unsigned frames{};
  unsigned failedLoads{};
  unsigned loadAttempts{};
};
