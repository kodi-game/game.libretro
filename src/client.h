/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro/ClientBridge.h"
#include "libretro/LibretroDLL.h"
#include "utils/Timer.h"

#include <kodi/addon-instance/Game.h>

namespace LIBRETRO
{
  class CGameInfoLoader;
}

class ATTRIBUTE_HIDDEN CGameLibRetro
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceGame
{
public:
  CGameLibRetro();
  ~CGameLibRetro() override;

  ADDON_STATUS Create() override;
  ADDON_STATUS GetStatus() override;
  ADDON_STATUS SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue) override;

  // --- Game operations ---------------------------------------------------------

  GAME_ERROR LoadGame(const std::string& url) override;
  GAME_ERROR LoadGameSpecial(SPECIAL_GAME_TYPE type, const std::vector<std::string>& urls) override;
  GAME_ERROR LoadStandalone() override;
  GAME_ERROR UnloadGame() override;
  GAME_ERROR GetGameTiming(game_system_timing& timing_info) override;
  GAME_REGION GetRegion() override;
  bool RequiresGameLoop() override { return true; }
  GAME_ERROR RunFrame() override;
  GAME_ERROR Reset() override;

  // --- Hardware rendering operations -------------------------------------------

  GAME_ERROR HwContextReset() override;
  GAME_ERROR HwContextDestroy() override;

  // --- Input operations --------------------------------------------------------

  bool HasFeature(const std::string& controller_id, const std::string& feature_name) override;
  game_input_topology* GetTopology() override;
  void FreeTopology(game_input_topology* topology) override;
  void SetControllerLayouts(const std::vector<kodi::addon::GameControllerLayout>& controllers) override;
  bool EnableKeyboard(bool enable, const std::string& controller_id) override;
  bool EnableMouse(bool enable, const std::string& controller_id) override;
  bool ConnectController(bool connect, const std::string& port_address, const std::string& controller_id) override;
  bool InputEvent(const game_input_event& event) override;

  // --- Serialization operations ------------------------------------------------

  size_t SerializeSize() override;
  GAME_ERROR Serialize(uint8_t* data, size_t size) override;
  GAME_ERROR Deserialize(const uint8_t* data, size_t size) override;

  // --- Cheat operations --------------------------------------------------------

  GAME_ERROR CheatReset() override;
  GAME_ERROR GetMemory(GAME_MEMORY type, uint8_t*& data, size_t& size) override;
  GAME_ERROR SetCheat(unsigned int index, bool enabled, const std::string& code) override;

  // --- RCheevos ----------------------------------------------------------------
  GAME_ERROR RCGenerateHashFromFile(char* hash, int consoleID, const char* filePath) override;
  GAME_ERROR RCGetGameIDUrl(char* url, size_t size, const char* hash) override;
  GAME_ERROR RCGetPatchFileUrl(char* url,
                               size_t size,
                               const char* username,
                               const char* token,
                               unsigned gameID) override;
  GAME_ERROR SetRetroAchievementsCredentials(const char* username, const char* token);
  GAME_ERROR RCPostRichPresenceUrl(char* url,
                                   size_t urlSize,
                                   char* postData,
                                   size_t postSize,
                                   const char* username,
                                   const char* token,
                                   unsigned gameID,
                                   const char* richPresence) override;
  GAME_ERROR EnableRichPresence(const char* script) override;
  GAME_ERROR GetRichPresenceEvaluation(char* evaluation, size_t size) override;
  GAME_ERROR ActivateAchievement(unsigned cheevo_id, const char* memaddr) override;
  GAME_ERROR RCResetRuntime() override;
  GAME_ERROR GetCheevo_URL_ID(void (*Callback)(const char* achievement_url,
                                               unsigned cheevo_id)) override;

private:
  GAME_ERROR AudioAvailable();

  LIBRETRO::Timer                         m_timer;
  LIBRETRO::CLibretroDLL                  m_client;
  LIBRETRO::CClientBridge                 m_clientBridge;
  std::vector<LIBRETRO::CGameInfoLoader*> m_gameInfo;
  bool                                    m_supportsVFS = false; // TODO
  int64_t                                 m_frameTimeLast = 0;
};
