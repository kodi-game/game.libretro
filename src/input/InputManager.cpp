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

#include "libKODI_game.h"

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

void CInputManager::DeviceConnected(int port, bool bConnected, const game_controller* connectedDevice)
{
  if (bConnected)
    m_devices[port] = std::make_shared<CLibretroDevice>(connectedDevice);
  else
    m_devices[port].reset();
}

libretro_device_t CInputManager::GetDeviceType(unsigned int port) const
{
  libretro_device_t deviceType = RETRO_DEVICE_NONE;

  auto it = m_devices.find(port);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
    {
      libretro_subclass_t subclass = device->Subclass();
      if (subclass == RETRO_SUBCLASS_NONE)
        deviceType = device->Type();
      else
        deviceType = RETRO_DEVICE_SUBCLASS(device->Type(), subclass);
    }
  }

  return deviceType;
}

bool CInputManager::OpenPort(unsigned int port)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
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

      dsyslog("Key %s: %s (XBMCVKey: 0x%04x, RETROK: 0x%04x, Modifier: 0x%02x)", down ? "down" : "up",
          LibretroTranslator::GetKeyName(event.key.character), character, keycode, key_modifiers);

      clientBridge->KeyboardEvent(down, keycode, character, key_modifiers);
    }

    // Record key press for polling
    HandlePress(event.key);

    bHandled = true;
  }
  else
  {
    const int port = event.port;

    if (m_devices[port])
      bHandled = m_devices[port]->Input().InputEvent(event);
  }

  return bHandled;
}

void CInputManager::LogInputDescriptors(const retro_input_descriptor* descriptors) const
{
  dsyslog("Libretro input bindings:");
  dsyslog("------------------------------------------------------------");

  for (const retro_input_descriptor* descriptor = descriptors;
      descriptor != nullptr && descriptor->description != nullptr && !std::string(descriptor->description).empty();
      descriptor++)
  {
    std::string component = LibretroTranslator::GetComponentName(descriptor->device, descriptor->index, descriptor->id);

    if (component.empty())
    {
      dsyslog("Port: %u, Device: %s, Feature: %s, Description: %s",
          descriptor->port,
          LibretroTranslator::GetDeviceName(descriptor->device),
          LibretroTranslator::GetFeatureName(descriptor->device, descriptor->index, descriptor->id),
          descriptor->description ? descriptor->description : "");
    }
    else
    {
      dsyslog("Port: %u, Device: %s, Feature: %s, Component: %s, Description: %s",
          descriptor->port,
          LibretroTranslator::GetDeviceName(descriptor->device),
          LibretroTranslator::GetFeatureName(descriptor->device, descriptor->index, descriptor->id),
          component.c_str(),
          descriptor->description ? descriptor->description : "");
    }
  }

  dsyslog("------------------------------------------------------------");
}

std::string CInputManager::ControllerID(unsigned int port) const
{
  std::string controllerId;

  auto it = m_devices.find(port);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      controllerId = device->ControllerID();
  }

  return controllerId;
}

bool CInputManager::ButtonState(libretro_device_t device, unsigned int port, unsigned int buttonIndex) const
{
  bool bState = false;

  if (device == RETRO_DEVICE_KEYBOARD)
  {
    bState = IsPressed(buttonIndex);
  }
  else
  {
    int iPort = port;

    if (device == RETRO_DEVICE_MOUSE)
      iPort = GAME_INPUT_PORT_MOUSE;

    auto it = m_devices.find(iPort);
    if (it != m_devices.end())
    {
      const auto &device = it->second;
      if (device)
        bState = device->Input().ButtonState(buttonIndex);
    }
  }

  return bState;
}

float CInputManager::AnalogButtonState(unsigned int port, unsigned int buttonIndex) const
{
  float state = 0.0f;

  auto it = m_devices.find(port);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      state = device->Input().AnalogButtonState(buttonIndex);
  }

  return state;
}

int CInputManager::DeltaX(libretro_device_t device, unsigned int port)
{
  int deltaX = 0;

  int iPort = port;

  if (device == RETRO_DEVICE_MOUSE)
    iPort = GAME_INPUT_PORT_MOUSE;

  auto it = m_devices.find(iPort);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      deltaX = device->Input().RelativePointerDeltaX();
  }

  return deltaX;
}

int CInputManager::DeltaY(libretro_device_t device, unsigned int port)
{
  int deltaY = 0;

  int iPort = port;

  if (device == RETRO_DEVICE_MOUSE)
    iPort = GAME_INPUT_PORT_MOUSE;

  auto it = m_devices.find(iPort);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      deltaY = device->Input().RelativePointerDeltaY();
  }

  return deltaY;
}

bool CInputManager::AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y) const
{
  bool bSuccess = false;

  auto it = m_devices.find(port);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      bSuccess = device->Input().AnalogStickState(analogStickIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y) const
{
  bool bSuccess = false;

  auto it = m_devices.find(port);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      bSuccess = device->Input().AbsolutePointerState(pointerIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AccelerometerState(unsigned int port, float& x, float& y, float& z) const
{
  bool bSuccess = false;

  auto it = m_devices.find(port);
  if (it != m_devices.end())
  {
    const auto &device = it->second;
    if (device)
      bSuccess = device->Input().AccelerometerState(x, y, z);
  }

  return bSuccess;
}

void CInputManager::SetControllerInfo(const retro_controller_info* info)
{
  dsyslog("Libretro controller info:");
  dsyslog("------------------------------------------------------------");

  for (unsigned int i = 0; i < info->num_types; i++)
  {
    const retro_controller_description& type = info->types[i];

    libretro_device_t baseType = type.id & RETRO_DEVICE_MASK;

    std::string description = type.desc ? type.desc : "";

    if (type.id & ~RETRO_DEVICE_MASK)
    {
      libretro_subclass_t subclass = (type.id >> RETRO_DEVICE_TYPE_SHIFT) - 1;
      dsyslog("Device: %s, Subclass: %u, Description: \"%s\"",
          LibretroTranslator::GetDeviceName(baseType), subclass, description.c_str());
    }
    else
    {
      dsyslog("Device: %s, Description: \"%s\"",
          LibretroTranslator::GetDeviceName(baseType), description.c_str());
    }
  }

  dsyslog("------------------------------------------------------------");
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

bool CInputManager::IsPressed(uint32_t character) const
{
  CLockObject lock(m_keyMutex);

  return std::count_if(m_pressedKeys.begin(), m_pressedKeys.end(),
    [character](const game_key_event& keyEvent)
    {
      return keyEvent.character == character;
    }) > 0;
}
