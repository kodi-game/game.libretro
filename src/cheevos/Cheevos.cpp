/*
 *  This file is based on https://github.com/libretro/RetroArch/blob/96c5f5dfb07454c972c50838815330382d6b1911/cheevos/fixup.c
 *
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

constexpr int URL_SIZE = 512;

CCheevos::CCheevos()
{
  rc_runtime_init(&m_runtime);
  m_cheevo_id = 0;
  m_url = NULL;
}

void CCheevos::Initialize()
{
  rc_runtime_init(&m_runtime);
}

void CCheevos::Deinitialize()
{
  rc_runtime_destroy(&m_runtime);
}

void CCheevos::ResetRuntime()
{
  rc_runtime_reset(&m_runtime);
  m_richPresence = rc_parse_richpresence(m_richPresenceBuffer.data(), m_richPresenceScript.c_str(), NULL, 0);
}

CCheevos& CCheevos::Get(void)
{
  static CCheevos _instance;
  return _instance;
}

bool CCheevos::GenerateHashFromFile(char* hash, int consoleID, const char* filePath)
{
  int res = rc_hash_generate_from_file(hash, consoleID, filePath);
  m_hash = hash;
  return res != 0;
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

void CCheevos::SetRetroAchievementsCredentials(const char* username, const char* token)
{
  m_username = username;
  m_token = token;
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

  m_richPresence = rc_parse_richpresence(m_richPresenceBuffer.data(), script, NULL, 0);

  m_richPresenceScript = script;
}

void CCheevos::EvaluateRichPresence(char* evaluation, size_t size)
{
  rc_evaluate_richpresence(m_richPresence, evaluation, size, PeekInternal, this, NULL);
}

void CCheevos::ActivateAchievement(unsigned cheevo_id, const char* memaddr)
{
  rc_runtime_activate_achievement(&m_runtime, cheevo_id, memaddr, NULL, 0);
  // it will return integer value 0 in case achivement is activated successfully.
}

bool CCheevos::AwardAchievement(
    char* url, size_t size, unsigned cheevo_id, int hardcore, const char* game_hash)
{
  return rc_url_award_cheevo(url, size, m_username.c_str(), m_token.c_str(), cheevo_id, 0,
                             game_hash) >= 0;
}

void CCheevos::GetCheevo_URL_ID(void (*Callback)(const char* achievement_url, unsigned cheevo_id))
{
  this->Callback = Callback;
}

void CCheevos::DeactivateTriggeredAchievement(unsigned cheevo_id)
{
  rc_runtime_deactivate_achievement(&m_runtime, cheevo_id);
  char url[URL_SIZE];
  if (AwardAchievement(url, URL_SIZE, cheevo_id, 0, m_hash.c_str()))
  {
    std::string achievement_url = url;
    this->Callback(url, cheevo_id);
  }
}

void CCheevos::RuntimeEventHandler(const rc_runtime_event_t* runtime_event)
{
  if (runtime_event->type == RC_RUNTIME_EVENT_ACHIEVEMENT_TRIGGERED)
  {
    CCheevos::Get().DeactivateTriggeredAchievement(runtime_event->id);
  }
}

void CCheevos::TestCheevoStatusPerFrame()
{
  rc_runtime_do_frame(&m_runtime, &RuntimeEventHandler, PeekInternal, this, NULL);
}

unsigned int CCheevos::PeekInternal(unsigned address, unsigned num_bytes, void* ud)
{
  CCheevos* cheevos = static_cast<CCheevos*>(ud);
  if (cheevos != nullptr)
    return cheevos->Peek(address, num_bytes);

  return 0;
}

unsigned int CCheevos::Peek(unsigned int address, unsigned int numBytes)
{
  CMemoryMap mmap = CLibretroEnvironment::Get().GetMemoryMap();

  const uint8_t* data = FixupFind(address, mmap, 5);
  unsigned value = 0;

  if (data)
  {
    switch (numBytes)
    {
      case 4:
        value |= data[2] << 16 | data[3] << 24;
        //no break
      case 2:
        value |= data[1] << 8;
        //no break
      case 1:
        value |= data[0];
        //no break
      default:
        break;
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
