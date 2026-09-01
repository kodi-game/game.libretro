/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "libretro/ClientBridge.h"
#include "libretro/LibretroDLL.h"
#include "utils/Timer.h"

#include <kodi/addon-instance/Game.h>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace LIBRETRO
{
  class CGameInfoLoader;
}

class ATTR_DLL_LOCAL CGameLibRetro
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceGame
{
public:
  CGameLibRetro();
  ~CGameLibRetro() override;

  ADDON_STATUS Create() override;
  ADDON_STATUS SetSetting(const std::string& settingName, const kodi::addon::CSettingValue& settingValue) override;

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
  size_t AchievementStateSize() override;
  GAME_ERROR SerializeAchievements(uint8_t* data, size_t size) override;
  GAME_ERROR DeserializeAchievements(const uint8_t* data, size_t size) override;

  // --- Cheat operations --------------------------------------------------------

  GAME_ERROR CheatReset() override;
  GAME_ERROR GetMemory(GAME_MEMORY type, uint8_t*& data, size_t& size) override;
  GAME_ERROR SetCheat(unsigned int index, bool enabled, const std::string& code) override;

  // --- RCheevos ----------------------------------------------------------------
  GAME_ERROR SetRetroAchievementsCredentials(const std::string& username, const std::string& token) override;
#if defined(HAVE_GAME_RC_ENCORE_MODE)
  GAME_ERROR RCSetEncoreModeEnabled(bool enabled) override;
#endif
  GAME_ERROR ActivateAchievement(unsigned cheevo_id, const std::string& memAddrExpression) override;
  GAME_ERROR GetCheevoUrlId(const std::function<void(const std::string& achievementUrl,
                                                     unsigned int cheevoId)>& callback) override;

  // --- Disc control interface

  bool GetEjectState() override;
  GAME_ERROR SetEjectState(bool ejected) override;
  unsigned int GetImageIndex() override;
  GAME_ERROR SetImageIndex(unsigned int imageIndex) override;
  unsigned int GetImageCount() override;
  GAME_ERROR AddImageIndex() override;
  GAME_ERROR ReplaceImageIndex(unsigned int imageIndex, const std::string& filePath) override;
  GAME_ERROR RemoveImageIndex(unsigned int imageIndex) override;
  GAME_ERROR SetInitialImage(unsigned int imageIndex, const std::string& filePath) override;
  std::string GetImagePath(unsigned int imageIndex) override;
  std::string GetImageLabel(unsigned int imageIndex) override;

private:
  GAME_ERROR AudioAvailable();

  LIBRETRO::Timer                         m_timer;
  LIBRETRO::CLibretroDLL                  m_client;
  LIBRETRO::CClientBridge                 m_clientBridge;
  std::vector<LIBRETRO::CGameInfoLoader*> m_gameInfo;
  bool                                    m_supportsVFS = false; // TODO
  int64_t                                 m_frameTimeLast = 0;
};
