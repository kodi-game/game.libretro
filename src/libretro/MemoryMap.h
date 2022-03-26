/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <vector>
#include "libretro-common/libretro.h"

struct retro_memory_descriptor_kodi
{
  retro_memory_descriptor descriptor;
  size_t disconnectMask;
};

namespace LIBRETRO
{
class CMemoryMap
{
public:
  void Initialize(const retro_memory_map& mmap);
  const retro_memory_descriptor_kodi& operator[](int index) const;
  size_t Size() const;

private:
  bool PreprocessDescriptors();
  size_t AddBitsDown(size_t num);
  size_t Inflate(size_t addr, size_t mask);
  size_t Reduce(size_t addr, size_t mask);
  size_t HighestBit(size_t num);

  std::vector<retro_memory_descriptor_kodi> m_mmap;
};
} // namespace LIBRETRO
