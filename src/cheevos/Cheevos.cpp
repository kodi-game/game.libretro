/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Cheevos.h"
#include "libretro/libretro.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/MemoryMap.h"
#include "rcheevos/rconsoles.h"
#include "rcheevos/rhash.h"
#include "rcheevos/rurl.h"

using namespace LIBRETRO;

CCheevos::CCheevos()
{
  rc_runtime_init(&m_runtime);
}

CCheevos::~CCheevos()
{
  rc_runtime_destroy(&m_runtime);
}

void CCheevos::Initialize()
{
  rc_runtime_init(&m_runtime);
}

void CCheevos::Deinitialize()
{
  rc_runtime_destroy(&m_runtime);
}

CCheevos& CCheevos::Get(void)
{
  static CCheevos _instance;
  return _instance;
}

bool CCheevos::GenerateHashFromFile(char* hash, int consoleID, const char* filePath)
{
  return rc_hash_generate_from_file(hash, consoleID, filePath) != 0;
}

bool CCheevos::GetGameIDUrl(char* url, size_t size, const char* hash)
{
  return rc_url_get_gameid(url, size, hash) == 0;
}

bool CCheevos::GetPatchFileUrl(char* url, 
                              size_t size, 
                              const char* username, 
                              const char* token, 
                              unsigned gameID)
{
  return rc_url_get_patch(url, size, username, token, gameID) == 0;
}

bool CCheevos::PostRichPresenceUrl(char* url,
                                   size_t urlSize,
                                   char* postData,
                                   size_t postSize,
                                   const char* username,
                                   const char* token,
                                   unsigned gameID,
                                   const char* richPresence)
{
  return rc_url_ping(url, urlSize, postData, postSize, username, token, gameID, richPresence) >= 0;
}

void CCheevos::EnableRichPresence(const char* script)
{
  rc_runtime_activate_richpresence(&m_runtime, script, NULL, 0);

  m_richPresenceBuffer.resize(rc_richpresence_size(script));

  m_richPresence.reset(
      rc_parse_richpresence(m_richPresenceBuffer.data(), script, NULL, 0));
}

void CCheevos::EvaluateRichPresence(char* evaluation, size_t size)
{
  rc_evaluate_richpresence(m_richPresence.get(), evaluation, size, peek, this, NULL);
}

unsigned LIBRETRO::peek(unsigned address, unsigned num_bytes, void* ud)
{
  CCheevos* cheevos = (CCheevos*)ud;
  CMemoryMap mmap = CLibretroEnvironment::Get().GetMemoryMap();

  const uint8_t* data = cheevos->FixupFind(address, mmap, 5);
  unsigned value = 0;

  if (data)
  {
    switch (num_bytes)
    {
      case 4:
        value |= data[2] << 16 | data[3] << 24;
      case 2:
        value |= data[1] << 8;
      case 1:
        value |= data[0];
    }
  }

  return value;
}

const uint8_t* CCheevos::FixupFind(unsigned address, CMemoryMap& mmap, int console)
{
  auto location = m_addressFixups.find(address);
  if (location != m_addressFixups.end())
    return location->second;

  const uint8_t* dataAddress = PatchAddress(address, mmap, console);
  m_addressFixups[address] = dataAddress;

  return dataAddress;
}

const uint8_t* CCheevos::PatchAddress(unsigned address, CMemoryMap& mmap, int console)
{
  const void* pointer = NULL;
  unsigned original_address = address;

  switch (console)
  {
    case RC_CONSOLE_NINTENDO:
      if (address >= 0x0800 && address < 0x2000)
        address &= 0x07ff;
      break;
    case RC_CONSOLE_GAMEBOY_COLOR:
      if (address >= 0xe000 && address <= 0xfdff)
        address -= 0x2000;
      break;
    default:
      break;
  }

  if (mmap.Size() != 0)
  {
    switch (console)
    {
      case RC_CONSOLE_GAMEBOY_ADVANCE:
        if (address < 0x8000)
          address += 0x3000000;
        else
          address += 0x2000000 - 0x8000;
        break;
      case RC_CONSOLE_PC_ENGINE:
        if (address < 0x002000)
          address += 0x1f0000;
        else if (address < 0x012000)
          address += 0x100000 - 0x002000;
        else if (address < 0x042000)
          address += 0x0d0000 - 0x012000;
        else
          address += 0x1ee000 - 0x042000;
        break;
      case RC_CONSOLE_SUPER_NINTENDO:
        if (address < 0x020000)
          address += 0x7e0000;
        else
          address += 0x006000 - 0x020000;
        break;
      case RC_CONSOLE_SEGA_CD:
        if (address < 0x010000)
          address += 0xFF0000;
        else
          address += 0x80020000 - 0x010000;
        break;
      default:
        break;
    }

    for (int i = 0; i < mmap.Size(); i++)
    {
      const retro_memory_descriptor_kodi& desc = mmap[i];
      if (((desc.descriptor.start ^ address) & desc.descriptor.select) == 0)
      {
        pointer = desc.descriptor.ptr;
        address -= desc.descriptor.start;

        if (desc.disconnectMask)
          address = Reduse(address & desc.disconnectMask, desc.descriptor.disconnect);

        if (address >= desc.descriptor.len)
          address -= HighestBit(address);

        address += desc.descriptor.offset;

        break;
      }
    }
  }

  if (!pointer)
    return NULL;

  return (const uint8_t*)pointer + address;
}

size_t CCheevos::HighestBit(size_t n)
{
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;

  return n ^ (n >> 1);
}

size_t CCheevos::Reduse(size_t addr, size_t mask)
{
  while (mask)
  {
    size_t tmp = (mask - 1) & ~mask;
    addr = (addr & tmp) | ((addr >> 1) & ~tmp);
    mask = (mask & (mask - 1)) >> 1;
  }

  return addr;
}
