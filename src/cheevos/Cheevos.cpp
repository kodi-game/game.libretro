/*
 *  This file is based on https://github.com/libretro/RetroArch/blob/96c5f5dfb07454c972c50838815330382d6b1911/cheevos/fixup.c
 *
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Cheevos.h"

#include "libretro/LibretroEnvironment.h"
#include "libretro/MemoryMap.h"
#include "rcheevos/rconsoles.h"
#include "rcheevos/rhash.h"
#include "rcheevos/rurl.h"

using namespace LIBRETRO;

namespace
{
constexpr unsigned int HASH_SIZE = 33;
constexpr unsigned int URL_SIZE = 512;
constexpr unsigned int RICH_PRESENCE_EVAL_SIZE = 512;
constexpr unsigned int POST_DATA_SIZE = 1024;
} // namespace

CCheevos::CCheevos()
{
  rc_runtime_init(&m_runtime);
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
  m_richPresence =
      rc_parse_richpresence(m_richPresenceBuffer.data(), m_richPresenceScript.c_str(), NULL, 0);
}

CCheevos& CCheevos::Get(void)
{
  static CCheevos _instance;
  return _instance;
}

bool CCheevos::GenerateHashFromFile(std::string& hash,
                                    unsigned int consoleID,
                                    const std::string& filePath)
{
  char _hash[HASH_SIZE] = {};

  int res = rc_hash_generate_from_file(_hash, consoleID, filePath.c_str());
  hash = _hash;

  return res != RC_OK;
}

bool CCheevos::GetGameIDUrl(std::string& url, const std::string& hash)
{
  char _url[URL_SIZE] = {};

  int res = rc_url_get_gameid(_url, URL_SIZE, hash.c_str());
  url = _url;

  return res == 0;
}

bool CCheevos::GetPatchFileUrl(std::string& url,
                               const std::string& username,
                               const std::string& token,
                               unsigned int gameID)
{
  char _url[URL_SIZE] = {};

  int res = rc_url_get_patch(_url, URL_SIZE, username.c_str(), token.c_str(), gameID);
  url = _url;

  return res == 0;
}

void CCheevos::SetRetroAchievementsCredentials(const std::string& username, const std::string& token)
{
  m_username = username;
  m_token = token;
}

bool CCheevos::PostRichPresenceUrl(std::string& url,
                                   std::string& postData,
                                   const std::string& username,
                                   const std::string& token,
                                   unsigned int gameID,
                                   const std::string& richPresence)
{
  char _url[URL_SIZE] = {};
  char _postData[POST_DATA_SIZE] = {};

  int res = rc_url_ping(_url, URL_SIZE, _postData, POST_DATA_SIZE, username.c_str(), token.c_str(),
                        gameID, richPresence.c_str());
  url = _url;
  postData = _postData;

  return res >= 0;
}

void CCheevos::EnableRichPresence(const std::string& script)
{
  const char* _script = script.c_str();

  rc_runtime_activate_richpresence(&m_runtime, _script, NULL, 0);

  m_richPresenceBuffer.resize(rc_richpresence_size(_script));

  m_richPresence = rc_parse_richpresence(m_richPresenceBuffer.data(), _script, NULL, 0);

  m_richPresenceScript = script;
}

void CCheevos::EvaluateRichPresence(std::string& evaluation, unsigned int consoleID)
{
  char _evaluation[RICH_PRESENCE_EVAL_SIZE] = {};

  m_consoleID = consoleID;
  rc_evaluate_richpresence(m_richPresence, _evaluation, RICH_PRESENCE_EVAL_SIZE, PeekInternal, this,
                           NULL);
  evaluation = _evaluation;
}

void CCheevos::ActivateAchievement(unsigned cheevo_id, const char* memaddr)
{
  rc_runtime_activate_achievement(&m_runtime, cheevo_id, memaddr, NULL, 0);
  // it will return integer value 0 in case achivement is activated successfully.
}

bool CCheevos::AwardAchievement(
    char* url, size_t size, unsigned cheevo_id, int hardcore, const std::string& game_hash)
{
  return rc_url_award_cheevo(url, size, m_username.c_str(), m_token.c_str(), cheevo_id, 0,
                             game_hash.c_str()) >= 0;
}

void CCheevos::GetCheevo_URL_ID(void (*callback)(const char* achievement_url, unsigned cheevo_id))
{
  m_callback = callback;
}

void CCheevos::DeactivateTriggeredAchievement(unsigned cheevo_id)
{
  rc_runtime_deactivate_achievement(&m_runtime, cheevo_id);
  char url[URL_SIZE];
  if (AwardAchievement(url, URL_SIZE, cheevo_id, 0, m_hash.c_str()))
  {
    std::string achievement_url = url;
    m_callback(url, cheevo_id);
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

  const uint8_t* data = FixupFind(address, mmap, m_consoleID);
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

const uint8_t* CCheevos::PatchAddress(size_t address, CMemoryMap& mmap, int console)
{
  const void* pointer = NULL;
  size_t original_address = address;

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

    for (size_t i = 0; i < mmap.Size(); i++)
    {
      const retro_memory_descriptor_kodi& desc = mmap[i];
      if (((desc.descriptor.start ^ address) & desc.descriptor.select) == 0)
      {
        pointer = desc.descriptor.ptr;
        address -= desc.descriptor.start;

        if (desc.disconnectMask)
          address = Reduce(address & desc.disconnectMask, desc.descriptor.disconnect);

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

size_t CCheevos::Reduce(size_t addr, size_t mask)
{
  while (mask)
  {
    size_t tmp = (mask - 1) & ~mask;
    addr = (addr & tmp) | ((addr >> 1) & ~tmp);
    mask = (mask & (mask - 1)) >> 1;
  }

  return addr;
}
