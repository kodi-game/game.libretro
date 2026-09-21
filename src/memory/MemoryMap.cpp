/*
 *  Copyright (C) 2014-2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "MemoryMap.h"

#include <libretro.h>

#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

using namespace LIBRETRO;

namespace
{
size_t AddBitsDown(size_t value)
{
  for (unsigned shift = 1; shift < std::numeric_limits<size_t>::digits; shift *= 2)
    value |= value >> shift;
  return value;
}

size_t Inflate(size_t address, size_t mask)
{
  while (mask)
  {
    const size_t below = (mask - 1) & ~mask;
    address = ((address & ~below) << 1) | (address & below);
    mask &= mask - 1;
  }
  return address;
}

size_t Reduce(size_t address, size_t mask)
{
  while (mask)
  {
    const size_t below = (mask - 1) & ~mask;
    address = (address & below) | ((address >> 1) & ~below);
    mask = (mask & (mask - 1)) >> 1;
  }
  return address;
}

size_t HighestBit(size_t value)
{
  value = AddBitsDown(value);
  return value ^ (value >> 1);
}
} // namespace

bool CMemoryMap::Initialize(const retro_memory_map& map)
{
  if (map.num_descriptors != 0 && map.descriptors == nullptr)
    return false;

  try
  {
    std::vector<MemoryDescriptor> candidate;
    candidate.reserve(map.num_descriptors);
    for (unsigned i = 0; i < map.num_descriptors; ++i)
    {
      const auto& source = map.descriptors[i];
      MemoryDescriptor descriptor;
      descriptor.flags = source.flags;
      descriptor.data = static_cast<uint8_t*>(source.ptr);
      descriptor.offset = source.offset;
      descriptor.start = source.start;
      descriptor.select = source.select;
      descriptor.disconnect = source.disconnect;
      descriptor.length = source.len;
      descriptor.addressSpace = source.addrspace ? source.addrspace : "";
      candidate.emplace_back(std::move(descriptor));
    }

    if (!Normalize(candidate))
      return false;

    m_descriptors.swap(candidate);
    return true;
  }
  catch (const std::bad_alloc&)
  {
    return false;
  }
  catch (const std::length_error&)
  {
    return false;
  }
}

bool CMemoryMap::Normalize(std::vector<MemoryDescriptor>& descriptors)
{
  // Intentionally mirrors RetroArch runloop.c's mmap_preprocess_descriptors()
  // and mmap_{add_bits_down,inflate,reduce,highest_bit} (audited 2026-09-19),
  // including the highest_reachable fix in 91bf7b553acedaa9f8b31b204c18448c0a6ff823.
  // Keep these semantics aligned; reject unrepresentable ranges instead of
  // letting their arithmetic wrap. See tests/MemoryModel.md for the audit.
  constexpr size_t maximum = std::numeric_limits<size_t>::max();
  size_t topAddress = 1;
  for (const auto& descriptor : descriptors)
  {
    if (descriptor.select != 0)
      topAddress |= descriptor.select;
    else
    {
      if (descriptor.length == 0 || descriptor.start > maximum - (descriptor.length - 1))
        return false;
      topAddress |= descriptor.start + (descriptor.length - 1);
    }
  }
  topAddress = AddBitsDown(topAddress);

  for (auto& descriptor : descriptors)
  {
    if (descriptor.select == 0)
    {
      if ((descriptor.length & (descriptor.length - 1)) != 0)
        return false;
      descriptor.select = topAddress &
                          ~Inflate(AddBitsDown(descriptor.length - 1), descriptor.disconnect);
    }

    if (descriptor.length == 0)
    {
      const size_t mask = AddBitsDown(Reduce(topAddress & ~descriptor.select,
                                           descriptor.disconnect));
      if (mask == maximum)
        return false; // The inferred length would be SIZE_MAX + 1.
      descriptor.length = mask + 1;
    }

    if (descriptor.start & ~descriptor.select)
      return false;
    if (descriptor.data && descriptor.offset > maximum - (descriptor.length - 1))
      return false; // The last byte's buffer offset must be representable.

    const size_t highestReachable = Inflate(descriptor.length - 1, descriptor.disconnect);
    while (HighestBit(topAddress & ~descriptor.select & ~descriptor.disconnect) >
           HighestBit(highestReachable))
    {
      descriptor.disconnect |= HighestBit(topAddress & ~descriptor.select & ~descriptor.disconnect);
    }
  }
  return true;
}
