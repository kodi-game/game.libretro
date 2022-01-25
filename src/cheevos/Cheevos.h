/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "rcheevos/rcheevos.h"

#include <memory>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace LIBRETRO
{
class CMemoryMap;

class CCheevos
{
public:
  CCheevos();
  static CCheevos& Get();
  void Initialize();
  void Deinitialize();
  // When the game is reset, the runtime should also be reset
  void ResetRuntime();
  bool GenerateHashFromFile(char* hash, int consoleID, const char* filePath);
  bool GetGameIDUrl(char* url, size_t size, const char* hash);
  void SetRetroAchievementsCredentials(const char* username, const char* token);
  bool GetPatchFileUrl(
      char* url, size_t size, const char* username, const char* token, unsigned gameID);
  bool PostRichPresenceUrl(char* url,
                           size_t urlSize,
                           char* postData,
                           size_t postSize,
                           const char* username,
                           const char* token,
                           unsigned gameID,
                           const char* richPresence);
  void EnableRichPresence(const char* script);
  void EvaluateRichPresence(char* evaluation, size_t size);
  void ActivateAchievement(unsigned cheevo_id, const char* memaddr);
  bool AwardAchievement(
      char* url, size_t size, unsigned cheevo_id, int hardcore, const char* game_hash);
  void DeactivateTriggeredAchievement(unsigned cheevo_id);
  void TestCheevoStatusPerFrame();
  unsigned int Peek(unsigned int address, unsigned int numBytes);
  void GetCheevo_URL_ID(void (*Callback)(const char* achievement_url, unsigned cheevo_id));

private:
  const uint8_t* FixupFind(unsigned address, CMemoryMap& mmap, int consolecheevos);
  const uint8_t* PatchAddress(unsigned address, CMemoryMap& mmap, int console);
  size_t HighestBit(size_t n);
  size_t Reduse(size_t addr, size_t mask);

  static unsigned int PeekInternal(unsigned address, unsigned num_bytes, void* ud);
  static void RuntimeEventHandler(const rc_runtime_event_t* runtime_event);

  void (*Callback)(const char* achievement_url, unsigned cheevo_id);

  rc_runtime_t m_runtime;
  std::unordered_map<unsigned, const uint8_t*> m_addressFixups;

  // Rich Presence
  rc_richpresence_t* m_richPresence = nullptr;
  std::string m_richPresenceScript;
  std::vector<char> m_richPresenceBuffer;

  std::string m_hash;
  std::string m_username;
  std::string m_token;
  char* m_url;
  unsigned m_cheevo_id;
};
}
