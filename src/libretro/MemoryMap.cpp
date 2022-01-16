/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "MemoryMap.h"

using namespace LIBRETRO;

void CMemoryMap::Initialize(const retro_memory_map& mmap)
{
  for (unsigned int i = 0; i < mmap.num_descriptors; i++)
    m_mmap.push_back({mmap.descriptors[i], 0});

  PreprocessDescriptors();
}

const retro_memory_descriptor_kodi& CMemoryMap::operator[](int index) const
{
  return m_mmap[index];
}

size_t CMemoryMap::Size() const
{
  return m_mmap.size();
}

bool CMemoryMap::PreprocessDescriptors()
{
  size_t top_addr = 1;
  for (auto& desc : m_mmap)
  {
    if (desc.descriptor.select != 0)
      top_addr |= desc.descriptor.select;
    else
      top_addr |= desc.descriptor.start + desc.descriptor.len - 1;
  }

  top_addr = AddBitsDown(top_addr);

  for (auto& desc : m_mmap)
  {
    if (desc.descriptor.select == 0)
    {
      if (desc.descriptor.len == 0)
        return false;

      if ((desc.descriptor.len & (desc.descriptor.len - 1)) != 0)
        return false;

      desc.descriptor.select =
          top_addr & ~Inflate(AddBitsDown(desc.descriptor.len - 1), desc.descriptor.disconnect);
    }

    if (desc.descriptor.len == 0)
    {
      desc.descriptor.len =
          AddBitsDown(Reduce(top_addr & ~desc.descriptor.select, desc.descriptor.disconnect)) + 1;
    }

    if (desc.descriptor.start & ~desc.descriptor.select)
      return false;

    while (Reduce(top_addr & ~desc.descriptor.select, desc.descriptor.disconnect) >> 1 >
           desc.descriptor.len - 1)
    {
      desc.descriptor.disconnect |=
          HighestBit(top_addr & ~desc.descriptor.select & ~desc.descriptor.disconnect);
    }

    desc.disconnectMask = AddBitsDown(desc.descriptor.len - 1);
    desc.descriptor.disconnect &= desc.disconnectMask;

    while ((~desc.disconnectMask) >> 1 & desc.descriptor.disconnect)
    {
      desc.disconnectMask >>= 1;
      desc.descriptor.disconnect &= desc.disconnectMask;
    }
  }

  return true;
}

size_t CMemoryMap::AddBitsDown(size_t num)
{
  num |= num >> 1;
  num |= num >> 2;
  num |= num >> 4;
  num |= num >> 8;
  num |= num >> 16;

  if (sizeof(size_t) > 4)
    num |= num >> 16 >> 16;

  return num;
}

size_t CMemoryMap::Inflate(size_t addr, size_t mask)
{
  while (mask)
  {
    size_t tmp = (mask - 1) & ~mask;
    addr = ((addr & ~tmp) << 1) | (addr & tmp);
    mask = mask & (mask - 1);
  }

  return addr;
}

size_t CMemoryMap::Reduce(size_t addr, size_t mask)
{
  while (mask)
  {
    size_t tmp = (mask - 1) & ~mask;
    addr = (addr & tmp) | ((addr >> 1) & ~tmp);
    mask = (mask & (mask - 1)) >> 1;
  }

  return addr;
}

size_t CMemoryMap::HighestBit(size_t num)
{
  num = AddBitsDown(num);
  return num ^ (num >> 1);
}
