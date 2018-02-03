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

bool CInputManager::EnableKeyboard(const game_controller &controller)
{
  bool bSuccess = false;

  DevicePtr device(new CLibretroDevice(controller));
  m_keyboard = std::move(device);
  bSuccess = true;

  return bSuccess;
}

void CInputManager::DisableKeyboard()
{
  m_keyboard.reset();
}

void CInputManager::DeviceConnected(int port, bool bConnected, const game_controller* connectedDevice)
{
  if (bConnected)
    m_devices[port] = std::make_shared<CLibretroDevice>(*connectedDevice);
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
  std::string controllerId = event.controller_id != nullptr ? event.controller_id : "";
  std::string feature = event.feature_name != nullptr ? event.feature_name : "";

  if (controllerId.empty() || feature.empty())
    return false;

  bool bHandled = false;

  switch (event.type)
  {
  case GAME_INPUT_EVENT_KEY:
  {
    if (m_keyboard)
      bHandled = m_keyboard->Input().InputEvent(event);

    break;
  }
  default:
  {
    const int port = event.port;

    if (m_devices[port])
      bHandled = m_devices[port]->Input().InputEvent(event);

    break;
  }
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

  switch (device)
  {
  case RETRO_DEVICE_KEYBOARD:
  {
    if (m_keyboard)
      bState = m_keyboard->Input().ButtonState(buttonIndex);

    break;
  }
  default:
  {
    int iPort = port;

    if (device == RETRO_DEVICE_MOUSE)
      iPort = GAME_INPUT_PORT_MOUSE;

    auto it = m_devices.find(iPort);
    if (it != m_devices.end())
    {
      const auto &controller = it->second;
      if (controller)
        bState = controller->Input().ButtonState(buttonIndex);
    }

    break;
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
