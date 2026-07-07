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
#include "rcheevos/rc_api_request.h"
#include "rcheevos/rc_api_runtime.h"
#include "rcheevos/rc_consoles.h"
#include "rcheevos/rc_hash.h"

#include <cstring>
#include <limits>

using namespace LIBRETRO;

namespace
{
constexpr unsigned int HASH_SIZE = 33;
constexpr unsigned int RICH_PRESENCE_EVAL_SIZE = 512;
constexpr unsigned int URL_SIZE = 512;
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

  if (consoleID > static_cast<unsigned int>(std::numeric_limits<int>::max()))
  {
    hash.clear();
    m_hash.clear();
    return false;
  }

  const int raConsoleID = static_cast<int>(consoleID);
  int res = rc_hash_generate_from_file(_hash, raConsoleID, filePath.c_str());
  const bool success = (res != 0);

  hash = _hash;
  if (success)
  {
    m_addressFixups.clear();
    m_hash = hash;
    m_consoleID = consoleID;
  }
  else
  {
    hash.clear();
    m_hash.clear();
  }

  return success;
}

bool CCheevos::GetGameIDUrl(std::string& url, const std::string& hash)
{
  url.clear();

  rc_api_resolve_hash_request_t params{};
  params.game_hash = hash.c_str();

  rc_api_request_t request{};
  const int res = rc_api_init_resolve_hash_request(&request, &params);
  if (res == RC_OK && request.url != nullptr)
  {
    url = request.url;
    if (request.post_data != nullptr)
    {
      if (url.find('?') != std::string::npos)
        url += std::string("&") + request.post_data;
      else
        url += std::string("?") + request.post_data;
    }
  }
  rc_api_destroy_request(&request);

  return res == RC_OK && !url.empty();
}

bool CCheevos::GetPatchFileUrl(std::string& url,
                               const std::string& username,
                               const std::string& token,
                               unsigned int gameID)
{
  url.clear();

  rc_api_fetch_game_data_request_t params{};
  params.username = username.c_str();
  params.api_token = token.c_str();
  params.game_id = gameID;

  rc_api_request_t request{};
  const int res = rc_api_init_fetch_game_data_request(&request, &params);
  if (res == RC_OK && request.url != nullptr)
  {
    url = request.url;
    if (request.post_data != nullptr)
    {
      if (url.find('?') != std::string::npos)
        url += std::string("&") + request.post_data;
      else
        url += std::string("?") + request.post_data;
    }
  }
  rc_api_destroy_request(&request);

  return res == RC_OK && !url.empty();
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
  url.clear();
  postData.clear();

  rc_api_ping_request_t params{};
  params.username = username.c_str();
  params.api_token = token.c_str();
  params.game_id = gameID;
  params.rich_presence = richPresence.c_str();

  rc_api_request_t request{};
  const int res = rc_api_init_ping_request(&request, &params);
  if (res == RC_OK && request.url != nullptr)
  {
    url = request.url;
    postData = request.post_data != nullptr ? request.post_data : "";
  }
  rc_api_destroy_request(&request);

  return res == RC_OK && !url.empty();
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

bool CCheevos::ActivateAchievement(unsigned cheevo_id, const std::string& memAddrExpression)
{
  // Returns 0 when the achievement is activated successfully
  return rc_runtime_activate_achievement(&m_runtime, cheevo_id, memAddrExpression.c_str(), NULL,
                                         0) == 0;
}

bool CCheevos::AwardAchievement(
    char* url, size_t size, unsigned cheevo_id, int hardcore, const std::string& game_hash)
{
  if (url == nullptr || size == 0)
    return false;

  url[0] = '\0';

  rc_api_award_achievement_request_t params{};
  params.username = m_username.c_str();
  params.api_token = m_token.c_str();
  params.achievement_id = cheevo_id;
  params.hardcore = hardcore;
  params.game_hash = game_hash.c_str();

  rc_api_request_t request{};
  const int res = rc_api_init_award_achievement_request(&request, &params);
  if (res == RC_OK && request.url != nullptr)
  {
    std::string fullUrl = request.url;
    if (request.post_data != nullptr)
    {
      if (fullUrl.find('?') != std::string::npos)
        fullUrl += std::string("&") + request.post_data;
      else
        fullUrl += std::string("?") + request.post_data;
    }
    std::strncpy(url, fullUrl.c_str(), size - 1);
    url[size - 1] = '\0';
  }
  rc_api_destroy_request(&request);

  return res == RC_OK && url[0] != '\0';
}

void CCheevos::GetCheevoUrlId(const std::function<void(const std::string& achievementUrl,
                                                       unsigned int cheevoId)>& callback)
{
  m_callback = callback;
}

void CCheevos::DeactivateTriggeredAchievement(unsigned cheevo_id)
{
  rc_runtime_deactivate_achievement(&m_runtime, cheevo_id);

  if (m_hash.empty() || !m_callback)
    return;

  char url[URL_SIZE] = {};
  if (AwardAchievement(url, URL_SIZE, cheevo_id, 0, m_hash))
    m_callback(url, cheevo_id);
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

const uint8_t* CCheevos::FixupFind(unsigned address, CMemoryMap& mmap, unsigned int consoleID)
{
  auto location = m_addressFixups.find(address);
  if (location != m_addressFixups.end())
    return location->second;

  const uint8_t* dataAddress = PatchAddress(address, mmap, consoleID);
  m_addressFixups[address] = dataAddress;

  return dataAddress;
}

const uint8_t* CCheevos::PatchAddress(size_t address, CMemoryMap& mmap, unsigned int consoleID)
{
  const void* pointer = NULL;
  size_t original_address = address;

  switch (consoleID)
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
    switch (consoleID)
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
