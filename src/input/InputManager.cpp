/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "InputManager.h"
#include "ControllerTopology.h"
#include "LibretroDevice.h"
#include "LibretroDeviceInput.h"
#include "libretro/ClientBridge.h"
#include "libretro-common/libretro.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/LibretroTranslator.h"
#include "log/Log.h"

#include <algorithm>
#include <sstream>

using namespace LIBRETRO;

#define PORT_MAX_COUNT  32 // Large enough

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

void CInputManager::SetControllerLayouts(const std::vector<kodi::addon::GameControllerLayout>& controllers)
{
  m_controllerLayouts.clear();

  for (const auto &controller : controllers)
  {
    m_controllerLayouts[controller.controller_id].reset(new CControllerLayout(controller));
  }
}

bool CInputManager::EnableKeyboard(const std::string &controllerId)
{
  bool bSuccess = false;

  if (!CControllerTopology::GetInstance().SetDevice(GAME_PORT_KEYBOARD, controllerId))
  {
    esyslog("Error: Keyboard \"%s\" not supported", controllerId.c_str());
  }
  else
  {
    DevicePtr device(new CLibretroDevice(controllerId));
    m_keyboard = std::move(device);
    bSuccess = true;
  }

  return bSuccess;
}

void CInputManager::DisableKeyboard()
{
  CControllerTopology::GetInstance().RemoveDevice(GAME_PORT_KEYBOARD);
  m_keyboard.reset();
}

bool CInputManager::EnableMouse(const std::string &controllerId)
{
  bool bSuccess = false;

  if (!CControllerTopology::GetInstance().SetDevice(GAME_PORT_MOUSE, controllerId))
  {
    esyslog("Error: Mouse \"%s\" not supported", controllerId.c_str());
  }
  else
  {
    DevicePtr device(new CLibretroDevice(controllerId));
    m_mouse = std::move(device);
    bSuccess = true;
  }

  return bSuccess;
}

void CInputManager::DisableMouse()
{
  CControllerTopology::GetInstance().RemoveDevice(GAME_PORT_MOUSE);
  m_mouse.reset();
}

libretro_device_t CInputManager::ConnectController(const std::string &address, const std::string &controllerId)
{
  unsigned int deviceType = RETRO_DEVICE_NONE; // Unmasked device type

  const int port = GetPortIndex(address);
  if (port < 0)
  {
    esyslog("Failed to connect controller, invalid port address: %s", address.c_str());
  }
  else if (!controllerId.empty())
  {
    auto it = m_controllerLayouts.find(controllerId);
    if (it != m_controllerLayouts.end())
    {
      const CControllerLayout &controller = *it->second;

      const bool bProvidesInput = controller.ProvidesInput();

      if (!CControllerTopology::GetInstance().SetController(address, controllerId, bProvidesInput))
      {
        esyslog("Error: Controller port \"%s\" (libretro port %d) does not accept %s",
            address.c_str(), port, controllerId.c_str());
      }
      else
      {
        DevicePtr device(new CLibretroDevice(controllerId));

        // Check topology for type/subclass override
        libretro_device_t typeOverride = CControllerTopology::GetInstance().TypeOverride(address, controllerId);
        libretro_subclass_t subclassOverride = CControllerTopology::GetInstance().SubclassOverride(address, controllerId);

        if (typeOverride != RETRO_DEVICE_NONE)
          device->SetType(typeOverride);
        if (subclassOverride != RETRO_SUBCLASS_NONE)
          device->SetSubclass(typeOverride);

        // Calculate type value to send to libretro
        if (device->Subclass() != RETRO_SUBCLASS_NONE)
          deviceType = RETRO_DEVICE_SUBCLASS(device->Type(), device->Subclass());
        else
          deviceType = device->Type();

        if (port >= static_cast<int>(m_controllers.size()))
          m_controllers.resize(port + 1);

        m_controllers[port] = std::move(device);
      }
    }
  }

  return deviceType;
}

bool CInputManager::DisconnectController(const std::string &address)
{
  bool bSuccess = false;

  const int port = GetPortIndex(address);
  if (port < 0)
  {
    esyslog("Failed to disconnect controller, invalid port address: %s", address.c_str());
  }
  else
  {
    CControllerTopology::GetInstance().RemoveController(address);

    if (port < static_cast<int>(m_controllers.size()))
      m_controllers[port].reset();

    bSuccess = true;
  }

  return bSuccess;
}

int CInputManager::GetPortIndex(const std::string &address) const
{
  return CControllerTopology::GetInstance().GetPortIndex(address);
}

bool CInputManager::GetConnectionPortIndex(const std::string &address, int& connectionPort) const
{
  return CControllerTopology::GetInstance().GetConnectionPortIndex(address, connectionPort);
}

std::string CInputManager::GetAddress(unsigned int port) const
{
  return CControllerTopology::GetInstance().GetAddress(port);
}

libretro_device_t CInputManager::GetDeviceType(const std::string &address) const
{
  libretro_device_t deviceType = RETRO_DEVICE_NONE;

  int port = GetPortIndex(address);
  if (0 <= port && port < static_cast<int>(m_controllers.size()))
  {
    const DevicePtr &device = m_controllers[port];

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

void CInputManager::ClosePorts(void)
{
  m_controllers.clear();
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

  switch (event.port_type)
  {
  case GAME_PORT_KEYBOARD:
  {
    if (m_keyboard)
      bHandled |= m_keyboard->Input().InputEvent(event);

    break;
  }
  case GAME_PORT_MOUSE:
  {
    if (m_mouse)
      bHandled = m_mouse->Input().InputEvent(event);

    break;
  }
  case GAME_PORT_CONTROLLER:
  {
    std::string portAddress = event.port_address != nullptr ? event.port_address : "";
    int port = GetPortIndex(portAddress);

    if (0 <= port && port < PORT_MAX_COUNT)
    {
      // Resize devices if necessary
      if (port >= static_cast<int>(m_controllers.size()))
        m_controllers.resize(port + 1);

      if (m_controllers[port])
        bHandled = m_controllers[port]->Input().InputEvent(event);
      else
        esyslog("Event from controller %s sent to port with no device!",
            event.controller_id != nullptr ? event.controller_id : "");
    }

    break;
  }
  default:
    break;
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

  if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
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
  case RETRO_DEVICE_MOUSE:
  {
    if (m_mouse)
    {
      bState = m_mouse->Input().ButtonState(buttonIndex);
      break;
    }

    // Fall through to controller ports
  }
  default:
  {
    if (port < m_controllers.size())
    {
      const DevicePtr &controller = m_controllers[port];
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

  if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
    if (device)
      state = device->Input().AnalogButtonState(buttonIndex);
  }

  return state;
}

int CInputManager::DeltaX(libretro_device_t device, unsigned int port)
{
  int deltaX = 0;

  if (device == RETRO_DEVICE_MOUSE && m_mouse)
  {
    deltaX = m_mouse->Input().RelativePointerDeltaX();
  }
  else if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
    if (device)
      deltaX = device->Input().RelativePointerDeltaX();
  }

  return deltaX;
}

int CInputManager::DeltaY(libretro_device_t device, unsigned int port)
{
  int deltaY = 0;

  if (device == RETRO_DEVICE_MOUSE && m_mouse)
  {
    deltaY = m_mouse->Input().RelativePointerDeltaY();
  }
  else if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
    if (device)
      deltaY = device->Input().RelativePointerDeltaY();
  }

  return deltaY;
}

bool CInputManager::AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y) const
{
  bool bSuccess = false;

  if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
    if (device)
      bSuccess = device->Input().AnalogStickState(analogStickIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y) const
{
  bool bSuccess = false;

  if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
    if (device)
      bSuccess = device->Input().AbsolutePointerState(pointerIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AccelerometerState(unsigned int port, float& x, float& y, float& z) const
{
  bool bSuccess = false;

  if (port < m_controllers.size())
  {
    const DevicePtr &device = m_controllers[port];
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
