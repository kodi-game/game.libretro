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

#include "input/ButtonMapper.h"
#include "input/ControllerTopology.h"
#include "input/InputManager.h"
#include "libretro/libretro.h"
#include "libretro/LibretroEnvironment.h"
#include "log/Log.h"
#include "log/LogAddon.h"
#include "settings/Settings.h"
#include "GameInfoLoader.h"

#include "client.h"

#include <set>
#include <string>
#include <vector>

using namespace LIBRETRO;

#define GAME_CLIENT_NAME_UNKNOWN      "Unknown libretro core"
#define GAME_CLIENT_VERSION_UNKNOWN   "0.0.0"

void SAFE_DELETE_GAME_INFO(std::vector<CGameInfoLoader*>& vec)
{
  for (std::vector<CGameInfoLoader*>::iterator it = vec.begin(); it != vec.end(); ++it)
    delete *it;
  vec.clear();
}

CGameLibRetro::CGameLibRetro()
{
}

CGameLibRetro::~CGameLibRetro()
{
  /* TODO
  m_clientBridge.AudioEnable(false);
  */

  CInputManager::Get().ClosePorts();

  m_client.retro_deinit();

  CControllerTopology::GetInstance().Clear();

  CLibretroEnvironment::Get().Deinitialize();

  CLog::Get().SetType(SYS_LOG_TYPE_CONSOLE);

  SAFE_DELETE_GAME_INFO(m_gameInfo);
}

ADDON_STATUS CGameLibRetro::Create()
{
  try
  {
    std::string dllPath = GameClientDllPath();
    if (dllPath.empty())
      throw ADDON_STATUS_UNKNOWN;

    if (!m_client.Load(dllPath))
    {
      esyslog("Failed to load %s", dllPath.c_str());
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    unsigned int version = m_client.retro_api_version();
    if (version != 1)
    {
      esyslog("Expected libretro api v1, found version %u", version);
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    // Environment must be initialized before calling retro_init()
    CLibretroEnvironment::Get().Initialize(this, &m_client, &m_clientBridge);

    CButtonMapper::Get().LoadButtonMap();
    CControllerTopology::GetInstance().LoadTopology();

    m_client.retro_init();

    // Log core info
    retro_system_info systemInfo = { };
    m_client.retro_get_system_info(&systemInfo);

    // VFS support is derived from need_fullpath. This property means that the
    // libretro cores requires a valid pathname. Conversely, if need_fullpath
    // is false, the core can load from memory.
    m_supportsVFS = !systemInfo.need_fullpath;

    std::string libraryName = systemInfo.library_name ? systemInfo.library_name : "";
    std::string libraryVersion = systemInfo.library_version ? systemInfo.library_version : "";
    std::string extensions = systemInfo.valid_extensions ? systemInfo.valid_extensions : "";

    dsyslog("CORE: ----------------------------------");
    dsyslog("CORE: Library name:    %s", libraryName.c_str());
    dsyslog("CORE: Library version: %s", libraryVersion.c_str());
    dsyslog("CORE: Extensions:      %s", extensions.c_str());
    dsyslog("CORE: Supports VFS:    %s", m_supportsVFS ? "true" : "false");
    dsyslog("CORE: ----------------------------------");

    // Reject invalid properties
    std::set<std::string> coreExtensions; // TODO: Parse string from libretro API
    std::set<std::string> addonExtensions; // TODO: Convert char** to set<string>

    if (coreExtensions != addonExtensions)
    {
      std::string strAddonExtensions;// = StringUtils::Join(addonExtensions, "|"); // TODO
      esyslog("CORE: Extensions don't match addon.xml: %s", strAddonExtensions.c_str());
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    if (SupportsVFS() != m_supportsVFS)
    {
      esyslog("CORE: VFS support doesn't match addon.xml: %s", SupportsVFS() ? "true" : "false");
      throw ADDON_STATUS_PERMANENT_FAILURE;
    }

    /* TODO
    // Initialize libretro's extended audio interface
    m_clientBridge.AudioEnable(true);
    */
  }
  catch (const ADDON_STATUS& status)
  {
    return status;
  }

  return GetStatus();
}

ADDON_STATUS CGameLibRetro::GetStatus()
{
  if (!CSettings::Get().IsInitialized())
    return ADDON_STATUS_NEED_SETTINGS;

  return ADDON_STATUS_OK;
}

ADDON_STATUS CGameLibRetro::SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue)
{
  if (settingName == "" || settingValue.empty())
    return ADDON_STATUS_UNKNOWN;

  CSettings::Get().SetSetting(settingName, settingValue);
  CLibretroEnvironment::Get().SetSetting(settingName, settingValue.GetString());

  return ADDON_STATUS_OK;
}

GAME_ERROR CGameLibRetro::LoadGame(const std::string& url)
{
  // Build info loader vector
  SAFE_DELETE_GAME_INFO(m_gameInfo);
  m_gameInfo.push_back(new CGameInfoLoader(url, m_supportsVFS));

  bool bResult = false;

  // Try to load via memory
  retro_game_info gameInfo;
  if (m_gameInfo[0]->Load())
  {
    m_gameInfo[0]->GetMemoryStruct(gameInfo);
    bResult = m_client.retro_load_game(&gameInfo);
  }

  if (!bResult)
  {
    // Fall back to loading via path
    m_gameInfo[0]->GetPathStruct(gameInfo);
    bResult = m_client.retro_load_game(&gameInfo);
  }

  return bResult ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR CGameLibRetro::LoadGameSpecial(SPECIAL_GAME_TYPE type, const std::vector<std::string>& urls)
{
  // TODO
  return GAME_ERROR_FAILED;
  /*
  retro_system_info info = { };
  m_client.retro_get_system_info(&info);
  const bool bSupportsVFS = !info.need_fullpath;

  // Build info loader vector
  SAFE_DELETE_GAME_INFO(m_gameInfo);
  for (const auto& url : urls)
    m_gameInfo.push_back(new CGameInfoLoader(url, bSupportsVFS));

  // Try to load via memory
  std::vector<retro_game_info> infoVec;
  infoVec.resize(urls.size());
  bool bLoadFromMemory = true;
  for (unsigned int i = 0; bLoadFromMemory && i < urls.size(); i++)
    bLoadFromMemory &= m_gameInfo[i]->GetMemoryStruct(infoVec[i]);
  if (bLoadFromMemory)
  {
    if (m_client.retro_load_game_special(type, infoVec.data(), urls.size()))
      return GAME_ERROR_NO_ERROR;
  }

  // Fall back to loading by path
  for (unsigned int i = 0; i < urls.size(); i++)
    m_gameInfo[i]->GetPathStruct(infoVec[i]);
  bool result = m_client.retro_load_game_special(type, infoVec.data(), urls.size());

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
  */
}

GAME_ERROR CGameLibRetro::LoadStandalone()
{
  if (!m_client.retro_load_game(nullptr))
    return GAME_ERROR_FAILED;

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CGameLibRetro::UnloadGame()
{
  GAME_ERROR error = GAME_ERROR_FAILED;

  m_client.retro_unload_game();

  CLibretroEnvironment::Get().CloseStreams();

  error = GAME_ERROR_NO_ERROR;

  SAFE_DELETE_GAME_INFO(m_gameInfo);

  return error;
}

GAME_ERROR CGameLibRetro::GetGameTiming(game_system_timing& timing_info)
{
  retro_system_av_info retro_info = { };
  m_client.retro_get_system_av_info(&retro_info);

  timing_info.fps = retro_info.timing.fps;
  timing_info.sample_rate = retro_info.timing.sample_rate;

  // Report info to CLibretroEnvironment
  CLibretroEnvironment::Get().UpdateVideoGeometry(retro_info.geometry);

  return GAME_ERROR_NO_ERROR;
}

GAME_REGION CGameLibRetro::GetRegion()
{
  return m_client.retro_get_region() == RETRO_REGION_NTSC ? GAME_REGION_NTSC : GAME_REGION_PAL;
}

GAME_ERROR CGameLibRetro::RunFrame()
{
  // Trigger the frame time callback before running the core.
  uint64_t current = m_timer.microseconds();
  int64_t delta = 0;

  if (m_frameTimeLast > 0)
    delta = current - m_frameTimeLast;

  m_frameTimeLast = current;
  m_clientBridge.FrameTime(delta);

  m_client.retro_run();

  CLibretroEnvironment::Get().OnFrameEnd();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CGameLibRetro::Reset()
{
  m_client.retro_reset();

  return GAME_ERROR_NO_ERROR;
}

/*!
 * \Brief Notify a core about audio being available for writing
 *
 * This enables finer-grained audio control by allowing the frontend to
 * control when audio data is sent to the frontend.
 *
 * When this function is called, audio data should be written to the
 * frontend via the AddStreamData() Game API callback in the same thread
 * that invoked AudioAvailable().
 *
 * This extended interface is not recommended for use with emulators which
 * have highly synchronous audio.
 *
 * This function is not part of the Game API yet. It has been implemented
 * here in case a libretro core requires the extended audio interface.
 */
GAME_ERROR CGameLibRetro::AudioAvailable()
{
  return m_clientBridge.AudioAvailable();
}

GAME_ERROR CGameLibRetro::HwContextReset()
{
  return m_clientBridge.HwContextReset();
}

GAME_ERROR CGameLibRetro::HwContextDestroy()
{
  return m_clientBridge.HwContextDestroy();
}

bool CGameLibRetro::HasFeature(const std::string& controller_id, const std::string& feature_name)
{
  return CButtonMapper::Get().GetLibretroIndex(controller_id, feature_name) >= 0;
}

game_input_topology* CGameLibRetro::GetTopology()
{
  return CControllerTopology::GetInstance().GetTopology();
}

void CGameLibRetro::FreeTopology(game_input_topology* topology)
{
  CControllerTopology::FreeTopology(topology);
}

void CGameLibRetro::SetControllerLayouts(const std::vector<AddonGameControllerLayout>& controllers)
{
  CInputManager::Get().SetControllerLayouts(controllers);
}

bool CGameLibRetro::EnableKeyboard(bool enable, const std::string& controller_id)
{
  bool bSuccess = false;

  if (enable)
  {
    bSuccess = CInputManager::Get().EnableKeyboard(controller_id);
  }
  else
  {
    CInputManager::Get().DisableKeyboard();
    bSuccess = true;
  }

  return bSuccess;
}

bool CGameLibRetro::EnableMouse(bool enable, const std::string& controller_id)
{
  bool bSuccess = false;

  if (enable)
  {
    bSuccess = CInputManager::Get().EnableMouse(controller_id);
  }
  else
  {
    CInputManager::Get().DisableMouse();
    bSuccess = true;
  }

  return bSuccess;
}

bool CGameLibRetro::ConnectController(bool connect, const std::string& port_address, const std::string& controller_id)
{
  std::string strPortAddress(port_address);
  std::string strController;

  if (connect)
    strController = controller_id;

  const int port = CInputManager::Get().GetPortIndex(strPortAddress);
  if (port < 0)
  {
    esyslog("Failed to connect controller, invalid port address: %s", strPortAddress.c_str());
  }
  else
  {
    libretro_device_t device = RETRO_DEVICE_NONE;

    if (connect)
    {
      device = CInputManager::Get().ConnectController(strPortAddress, controller_id);
    }
    else
    {
      CInputManager::Get().DisconnectController(strPortAddress);
    }

    dsyslog("Setting port \"%s\" (libretro port %d) to controller \"%s\" (libretro device ID %u)",
        strPortAddress.c_str(), port, strController.c_str(), device);

    m_client.retro_set_controller_port_device(port, device);

    return true;
  }

  return false;
}

bool CGameLibRetro::InputEvent(const game_input_event& event)
{
  return CInputManager::Get().InputEvent(event);
}

size_t CGameLibRetro::SerializeSize()
{
  return m_client.retro_serialize_size();
}

GAME_ERROR CGameLibRetro::Serialize(uint8_t* data, size_t size)
{
  if (data == nullptr)
    return GAME_ERROR_INVALID_PARAMETERS;

  bool result = m_client.retro_serialize(data, size);

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR CGameLibRetro::Deserialize(const uint8_t* data, size_t size)
{
  if (data == nullptr)
    return GAME_ERROR_INVALID_PARAMETERS;

  bool result = m_client.retro_unserialize(data, size);

  return result ? GAME_ERROR_NO_ERROR : GAME_ERROR_FAILED;
}

GAME_ERROR CGameLibRetro::CheatReset()
{
  m_client.retro_cheat_reset();

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CGameLibRetro::GetMemory(GAME_MEMORY type, uint8_t*& data, size_t& size)
{
  data = static_cast<uint8_t*>(m_client.retro_get_memory_data(type));
  size = m_client.retro_get_memory_size(type);

  return GAME_ERROR_NO_ERROR;
}

GAME_ERROR CGameLibRetro::SetCheat(unsigned int index, bool enabled, const std::string& code)
{
  m_client.retro_cheat_set(index, enabled, code.c_str());

  return GAME_ERROR_NO_ERROR;
}

ADDONCREATOR(CGameLibRetro)
