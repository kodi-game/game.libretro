/*
 *      Copyright (C) 2014-2016 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
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
  void SetControllerLayouts(const std::vector<AddonGameControllerLayout>& controllers) override;
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

private:
  GAME_ERROR AudioAvailable();

  LIBRETRO::Timer                         m_timer;
  LIBRETRO::CLibretroDLL                  m_client;
  LIBRETRO::CClientBridge                 m_clientBridge;
  std::vector<LIBRETRO::CGameInfoLoader*> m_gameInfo;
  bool                                    m_supportsVFS = false; // TODO
  int64_t                                 m_frameTimeLast = 0;
};
