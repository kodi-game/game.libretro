/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Hash.h"

using namespace LIBRETRO;

uint64_t LIBRETRO::HashBytes(const void* data, size_t size, uint64_t hash)
{
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (size_t i = 0; i < size; ++i)
  {
    hash ^= bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}
