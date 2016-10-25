/*
 *      Copyright (C) 2015-2016 Team Kodi
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

#include "InputManager.h"
#include "LibretroDevice.h"
#include "LibretroDeviceInput.h"
#include "libretro/ClientBridge.h"
#include "libretro/libretro.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/LibretroTranslator.h"
#include "log/Log.h"

#include "kodi/libKODI_game.h"

#include <algorithm>

using namespace LIBRETRO;
using namespace P8PLATFORM;

CInputManager& CInputManager::Get(void)
{
  static CInputManager _instance;
  return _instance;
}

libretro_device_caps_t CInputManager::GetDeviceCaps(void) const
{
  return 1 << RETRO_DEVICE_JOYPAD   |
         1 << RETRO_DEVICE_MOUSE    |
         1 << RETRO_DEVICE_KEYBOARD |
         1 << RETRO_DEVICE_LIGHTGUN |
         1 << RETRO_DEVICE_ANALOG   |
         1 << RETRO_DEVICE_POINTER;
}

void CInputManager::DeviceConnected(unsigned int port, bool bConnected, const game_controller* connectedDevice)
{
  if (bConnected)
    m_devices[port] = std::make_shared<CLibretroDevice>(connectedDevice);
  else
    m_devices[port].reset();
}

libretro_device_t CInputManager::GetDevice(unsigned int port)
{
  libretro_device_t deviceType = 0;

  if (m_devices[port])
    deviceType = m_devices[port]->Type();

  return deviceType;
}

bool CInputManager::OpenPort(unsigned int port)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
    return false;

  // Sanity check
  if (port > 32)
    return false;

  CLibretroEnvironment::Get().GetFrontend()->OpenPort(port);

  return true;
}

DevicePtr CInputManager::GetPort(unsigned int port)
{
  return m_devices[port];
}

void CInputManager::ClosePort(unsigned int port)
{
  if (CLibretroEnvironment::Get().GetFrontend())
    CLibretroEnvironment::Get().GetFrontend()->ClosePort(port);

  m_devices[port].reset();
}

void CInputManager::ClosePorts(void)
{
  std::vector<unsigned int> ports;
  for (auto it = m_devices.begin(); it != m_devices.end(); ++it)
  {
    if (it->second)
      ports.push_back(it->first);
  }

  for (auto port : ports)
    ClosePort(port);
}

void CInputManager::EnableAnalogSensors(unsigned int port, bool bEnabled)
{
  // TODO
}

bool CInputManager::InputEvent(const game_input_event& event)
{
  bool bHandled = false;

  if (event.type == GAME_INPUT_EVENT_KEY)
  {
    // Report key to client
    CClientBridge* clientBridge = CLibretroEnvironment::Get().GetClientBridge();
    if (clientBridge)
    {
      const bool      down          = event.key.pressed;
      const retro_key keycode       = LibretroTranslator::GetKeyCode(event.key.character);
      const uint32_t  character     = event.key.character;
      const retro_mod key_modifiers = LibretroTranslator::GetKeyModifiers(event.key.modifiers);

      dsyslog("Key %s: %s (0x%04x)", down ? "down" : "up",
          LibretroTranslator::GetKeyName(character), character);

      clientBridge->KeyboardEvent(down, keycode, character, key_modifiers);
    }

    // Record key press for polling
    HandlePress(event.key);

    bHandled = true;
  }
  else
  {
    const unsigned int port = event.port;

    if (m_devices[port])
      bHandled = m_devices[port]->Input().InputEvent(event);
  }

  return bHandled;
}

void CInputManager::LogInputDescriptors(const retro_input_descriptor* descriptors)
{
  /* TODO
  for (const retro_input_descriptor* descriptor = descriptors; descriptor->description != NULL; descriptor++)
  {
    switch (descriptor->device)
    {
    case RETRO_DEVICE_JOYPAD:
    case RETRO_DEVICE_MOUSE:
    case RETRO_DEVICE_KEYBOARD:
    case RETRO_DEVICE_LIGHTGUN:
    case RETRO_DEVICE_ANALOG:
    case RETRO_DEVICE_POINTER:
    case RETRO_DEVICE_JOYPAD_MULTITAP:
    case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE:
    case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:
    case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS:
    default:
      break;
    }
  }
  */
}

std::string CInputManager::ControllerID(unsigned int port)
{
  std::string controllerId;

  if (m_devices[port])
    controllerId = m_devices[port]->ControllerID();

  return controllerId;
}

bool CInputManager::ButtonState(libretro_device_t device, unsigned int port, unsigned int buttonIndex)
{
  bool bState = false;

  if (device == RETRO_DEVICE_KEYBOARD)
  {
    bState = IsPressed(buttonIndex);
  }
  else
  {
    if (m_devices[port])
    {
      bState = m_devices[port]->Input().ButtonState(buttonIndex);
    }
  }

  return bState;
}

int CInputManager::DeltaX(libretro_device_t device, unsigned int port)
{
  int deltaX = 0;

  if (m_devices[port])
  {
    deltaX = m_devices[port]->Input().RelativePointerDeltaX();
  }

  return deltaX;
}

int CInputManager::DeltaY(libretro_device_t device, unsigned int port)
{
  int deltaY = 0;

  if (m_devices[port])
  {
    deltaY = m_devices[port]->Input().RelativePointerDeltaY();
  }

  return deltaY;
}

bool CInputManager::AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y)
{
  bool bSuccess = false;

  if (m_devices[port])
  {
    bSuccess = m_devices[port]->Input().AnalogStickState(analogStickIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y)
{
  bool bSuccess = false;

  if (m_devices[port])
  {
    bSuccess = m_devices[port]->Input().AbsolutePointerState(pointerIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AccelerometerState(unsigned int port, float& x, float& y, float& z)
{
  bool bSuccess = false;

  if (m_devices[port])
  {
    bSuccess = m_devices[port]->Input().AccelerometerState(x, y, z);
  }

  return bSuccess;
}

void CInputManager::HandlePress(const game_key_event& key)
{
  CLockObject lock(m_keyMutex);

  if (key.pressed)
  {
    m_pressedKeys.push_back(key);
  }
  else
  {
    m_pressedKeys.erase(std::remove_if(m_pressedKeys.begin(), m_pressedKeys.end(),
      [&key](const game_key_event& pressedKey)
      {
        return pressedKey.character == key.character;
      }), m_pressedKeys.end());
  }
}

bool CInputManager::IsPressed(uint32_t character)
{
  CLockObject lock(m_keyMutex);

  return std::count_if(m_pressedKeys.begin(), m_pressedKeys.end(),
    [character](const game_key_event& keyEvent)
    {
      return keyEvent.character == character;
    }) > 0;
}
