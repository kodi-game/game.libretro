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
  bool GenerateHashFromFile(std::string& hash, unsigned int consoleID, const std::string& filePath);
  bool GetGameIDUrl(std::string& url, const std::string& hash);
  bool GetPatchFileUrl(std::string& url,
                       const std::string& username,
                       const std::string& token,
                       unsigned int gameID);
  bool PostRichPresenceUrl(std::string& url,
                           std::string& postData,
                           const std::string& username,
                           const std::string& token,
                           unsigned int gameID,
                           const std::string& richPresence);
  void EnableRichPresence(const std::string& script);
  void EvaluateRichPresence(std::string& evaluation, unsigned int consoleID);
  unsigned int Peek(unsigned int address, unsigned int numBytes);

private:
  const uint8_t* FixupFind(unsigned address, CMemoryMap& mmap, int consolecheevos);
  const uint8_t* PatchAddress(size_t address, CMemoryMap& mmap, int consoleID);
  size_t HighestBit(size_t n);
  size_t Reduce(size_t addr, size_t mask);

  static unsigned int PeekInternal(unsigned address, unsigned num_bytes, void* ud);

  int m_consoleID;

  rc_runtime_t m_runtime;
  std::unordered_map<unsigned, const uint8_t*> m_addressFixups;

  // Rich Presence
  rc_richpresence_t* m_richPresence = nullptr;
  std::string m_richPresenceScript;
  std::vector<char> m_richPresenceBuffer;
};
} // namespace LIBRETRO
