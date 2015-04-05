/*
 *      Copyright (C) 2015 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "InputManager.h"
#include "libretro.h"
#include "LibretroEnvironment.h"

#include "kodi/libKODI_game.h"

#include <cstring>

using namespace LIBRETRO;
using namespace PLATFORM;

// --- CLibretroDevice ---------------------------------------------------------

CLibretroDevice::CLibretroDevice(const game_input_device* device /* = NULL */)
{
  if (!device)
  {
    m_type = RETRO_DEVICE_NONE;
  }
  else
  {
    m_type = RETRO_DEVICE_NONE; // TODO
  }
}

CLibretroDevice& CLibretroDevice::operator=(const CLibretroDevice& rhs)
{
  if (this != &rhs)
  {
    m_type             = rhs.m_type;
    m_digitalButtons   = rhs.m_digitalButtons;
    m_analogSticks     = rhs.m_analogSticks;
    m_accelerometers   = rhs.m_accelerometers;
    m_relativePointers = rhs.m_relativePointers;
    m_absolutePointers = rhs.m_absolutePointers;
  }
  return *this;
}

void CLibretroDevice::Clear(void)
{
  m_type = RETRO_DEVICE_NONE;
  m_digitalButtons.clear();
  m_analogSticks.clear();
  m_accelerometers.clear();
  m_relativePointers.clear();
  m_absolutePointers.clear();
}

bool CLibretroDevice::DigitalButtonState(unsigned int buttonIndex) const
{
  bool buttonState = false;

  if (buttonIndex < m_digitalButtons.size())
    buttonState = m_digitalButtons[buttonIndex].pressed;

  return buttonState;
}

bool CLibretroDevice::AnalogStickState(unsigned int analogStickIndex, float& x, float& y) const
{
  bool bSuccess = false;

  if (analogStickIndex < m_analogSticks.size())
  {
    x = m_analogSticks[analogStickIndex].x;
    y = m_analogSticks[analogStickIndex].y;
    bSuccess = true;
  }

  return bSuccess;
}

bool CLibretroDevice::AccelerometerState(float& x, float& y, float& z) const
{
  bool bSuccess = false;

  if (!m_accelerometers.empty())
  {
    x = m_accelerometers[0].x;
    y = m_accelerometers[0].y;
    z = m_accelerometers[0].z;
    bSuccess = true;
  }

  return bSuccess;
}

int CLibretroDevice::RelativePointerDeltaX(void)
{
  int deltaX = 0;

  if (!m_relativePointers.empty())
  {
    CLockObject lock(m_relativePtrMutex);

    deltaX = m_relativePointers[0].x;
    m_relativePointers[0].x = 0;
  }

  return deltaX;
}

int CLibretroDevice::RelativePointerDeltaY(void)
{
  int deltaY = 0;

  if (!m_relativePointers.empty())
  {
    CLockObject lock(m_relativePtrMutex);

    deltaY = m_relativePointers[0].y;
    m_relativePointers[0].y = 0;
  }

  return deltaY;
}

bool CLibretroDevice::AbsolutePointerState(unsigned int pointerIndex, float& x, float& y) const
{
  bool bSuccess = false;

  if (pointerIndex < m_absolutePointers.size() &&  m_absolutePointers[pointerIndex].pressed)
  {
    x = m_absolutePointers[pointerIndex].x;
    y = m_absolutePointers[pointerIndex].y;
    bSuccess = true;
  }

  return bSuccess;
}

bool CLibretroDevice::InputEvent(const game_input_event& event)
{
  const std::string strDeviceId = event.device_id;
  const std::string strFeatureName = event.feature_name;

  int index = GetLibretroIndex(strDeviceId, strFeatureName); // TODO: signed/unsigned comparison
  if (index >= 0)
  {
    switch (event.type)
    {
      case GAME_INPUT_EVENT_DIGITAL_BUTTON:
        if (index < m_digitalButtons.size())
          m_digitalButtons[index] = event.digital_button;
        break;

      case GAME_INPUT_EVENT_ANALOG_BUTTON:
        // TODO
        break;

      case GAME_INPUT_EVENT_ANALOG_STICK:
        if (index < m_analogSticks.size())
          m_analogSticks[index] = event.analog_stick;
        break;

      case GAME_INPUT_EVENT_ACCELEROMETER:
        if (index < m_accelerometers.size())
          m_accelerometers[index] = event.accelerometer;
        break;

      case GAME_INPUT_EVENT_KEY:
        // TODO
        break;

      case GAME_INPUT_EVENT_RELATIVE_POINTER:
        if (index < m_relativePointers.size())
        {
          CLockObject lock(m_relativePtrMutex);

          m_relativePointers[index].x += event.rel_pointer.x;
          m_relativePointers[index].y += event.rel_pointer.y;
        }
        break;

      case GAME_INPUT_EVENT_ABSOLUTE_POINTER:
        if (index < m_absolutePointers.size())
          m_absolutePointers[index] = event.abs_pointer;
        break;

      default:
        break;
    }

    return true;
  }

  return false;
}

int CLibretroDevice::GetLibretroIndex(const std::string& strDeviceId, const std::string& strFeatureName) const
{
  if (strDeviceId == "game.controller.default")
  {
    if (strFeatureName == "a")            return RETRO_DEVICE_ID_JOYPAD_A;
    if (strFeatureName == "b")            return RETRO_DEVICE_ID_JOYPAD_B;
    if (strFeatureName == "x")            return RETRO_DEVICE_ID_JOYPAD_X;
    if (strFeatureName == "y")            return RETRO_DEVICE_ID_JOYPAD_Y;
    if (strFeatureName == "start")        return RETRO_DEVICE_ID_JOYPAD_START;
    if (strFeatureName == "back")         return RETRO_DEVICE_ID_JOYPAD_SELECT;
    if (strFeatureName == "leftbumber")   return RETRO_DEVICE_ID_JOYPAD_L2;
    if (strFeatureName == "rightbumper")  return RETRO_DEVICE_ID_JOYPAD_R2;
    if (strFeatureName == "leftthumb")    return RETRO_DEVICE_ID_JOYPAD_L;
    if (strFeatureName == "rightthumb")   return RETRO_DEVICE_ID_JOYPAD_R;
    if (strFeatureName == "up")           return RETRO_DEVICE_ID_JOYPAD_UP;
    if (strFeatureName == "down")         return RETRO_DEVICE_ID_JOYPAD_DOWN;
    if (strFeatureName == "right")        return RETRO_DEVICE_ID_JOYPAD_RIGHT;
    if (strFeatureName == "left")         return RETRO_DEVICE_ID_JOYPAD_LEFT;
    if (strFeatureName == "lefttrigger")  return RETRO_DEVICE_ID_JOYPAD_L3;
    if (strFeatureName == "righttrigger") return RETRO_DEVICE_ID_JOYPAD_R3;
    if (strFeatureName == "leftstick")    return RETRO_DEVICE_INDEX_ANALOG_LEFT;
    if (strFeatureName == "rightstick")   return RETRO_DEVICE_INDEX_ANALOG_RIGHT;
  }
  else if (strDeviceId == "game.controller.nes")
  {
    if (strFeatureName == "a")            return RETRO_DEVICE_ID_JOYPAD_A;
    if (strFeatureName == "b")            return RETRO_DEVICE_ID_JOYPAD_B;
    if (strFeatureName == "start")        return RETRO_DEVICE_ID_JOYPAD_START;
    if (strFeatureName == "select")       return RETRO_DEVICE_ID_JOYPAD_SELECT;
    if (strFeatureName == "up")           return RETRO_DEVICE_ID_JOYPAD_UP;
    if (strFeatureName == "down")         return RETRO_DEVICE_ID_JOYPAD_DOWN;
    if (strFeatureName == "right")        return RETRO_DEVICE_ID_JOYPAD_RIGHT;
    if (strFeatureName == "left")         return RETRO_DEVICE_ID_JOYPAD_LEFT;
  }
  else if (strDeviceId == "game.controller.snes")
  {
    if (strFeatureName == "a")            return RETRO_DEVICE_ID_JOYPAD_A;
    if (strFeatureName == "b")            return RETRO_DEVICE_ID_JOYPAD_B;
    if (strFeatureName == "x")            return RETRO_DEVICE_ID_JOYPAD_X;
    if (strFeatureName == "y")            return RETRO_DEVICE_ID_JOYPAD_Y;
    if (strFeatureName == "start")        return RETRO_DEVICE_ID_JOYPAD_START;
    if (strFeatureName == "select")       return RETRO_DEVICE_ID_JOYPAD_SELECT;
    if (strFeatureName == "up")           return RETRO_DEVICE_ID_JOYPAD_UP;
    if (strFeatureName == "down")         return RETRO_DEVICE_ID_JOYPAD_DOWN;
    if (strFeatureName == "right")        return RETRO_DEVICE_ID_JOYPAD_RIGHT;
    if (strFeatureName == "left")         return RETRO_DEVICE_ID_JOYPAD_LEFT;
    if (strFeatureName == "rightbumper")  return RETRO_DEVICE_ID_JOYPAD_L2;
    if (strFeatureName == "leftbumper")   return RETRO_DEVICE_ID_JOYPAD_R2;
  }

  return -1;
}

// --- CInputManager -----------------------------------------------------------

CInputManager& CInputManager::Get(void)
{
  static CInputManager _instance;
  return _instance;
}

libretro_device_caps_t CInputManager::GetDeviceCaps(void) const
{
  libretro_device_caps_t deviceCaps = 0;

  deviceCaps |= (1 << RETRO_DEVICE_JOYPAD);
  //deviceCaps |= (1 << RETRO_DEVICE_MOUSE); // TODO
  //deviceCaps |= (1 << RETRO_DEVICE_KEYBOARD); // TODO
  //deviceCaps |= (1 << RETRO_DEVICE_LIGHTGUN); // TODO
  //deviceCaps |= (1 << RETRO_DEVICE_ANALOG); // TODO
  //deviceCaps |= (1 << RETRO_DEVICE_POINTER); // TODO

  return deviceCaps;
}

void CInputManager::DeviceConnected(unsigned int port, bool bConnected, const game_input_device* connectedDevice)
{
  if (port < m_ports.size())
    m_ports[port] = CLibretroDevice(bConnected ? connectedDevice : NULL);
}

libretro_device_t CInputManager::GetDevice(unsigned int port) const
{
  libretro_device_t deviceType = 0;

  if (port < m_ports.size())
    deviceType = m_ports[port].Type();

  return deviceType;
}

void CInputManager::ClosePorts(void)
{
  for (unsigned int i = 0; i < m_ports.size(); i++)
    ClosePort(i);
}

void CInputManager::EnableSource(bool bEnabled, unsigned int port, GAME_INPUT_EVENT_SOURCE source, unsigned int index)
{
  // TODO
}

bool CInputManager::InputEvent(unsigned int port, const game_input_event& event)
{
  bool bHandled = false;

  if (port < m_ports.size())
    bHandled = m_ports[port].InputEvent(event);

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

bool CInputManager::DigitalButtonState(libretro_device_t device, unsigned int port, unsigned int buttonIndex)
{
  bool bState = false;

  if (port < m_ports.size() || OpenPort(port))
  {
    bState = m_ports[port].DigitalButtonState(buttonIndex);
  }

  return bState;
}

int CInputManager::DeltaX(libretro_device_t device, unsigned int port)
{
  int deltaX = 0;

  if (port < m_ports.size() || OpenPort(port))
  {
    deltaX = m_ports[port].RelativePointerDeltaX();
  }

  return deltaX;
}

int CInputManager::DeltaY(libretro_device_t device, unsigned int port)
{
  int deltaY = 0;

  if (port < m_ports.size() || OpenPort(port))
  {
    deltaY = m_ports[port].RelativePointerDeltaY();
  }

  return deltaY;
}

bool CInputManager::AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y)
{
  bool bSuccess = false;

  if (port < m_ports.size() || OpenPort(port))
  {
    bSuccess = m_ports[port].AnalogStickState(analogStickIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y)
{
  bool bSuccess = false;

  if (port < m_ports.size() || OpenPort(port))
  {
    bSuccess = m_ports[port].AbsolutePointerState(pointerIndex, x, y);
  }

  return bSuccess;
}

bool CInputManager::AccelerometerState(unsigned int port, float& x, float& y, float& z)
{
  bool bSuccess = false;

  if (port < m_ports.size() || OpenPort(port))
  {
    bSuccess = m_ports[port].AccelerometerState(x, y, z);
  }

  return bSuccess;
}

bool CInputManager::OpenPort(unsigned int port)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
    return false;

  if (port >= m_ports.size())
    m_ports.resize(port + 1);

  CLibretroEnvironment::Get().GetFrontend()->OpenPort(port);

  return true;
}

void CInputManager::ClosePort(unsigned int port)
{
  if (!CLibretroEnvironment::Get().GetFrontend())
    return;

  CLibretroEnvironment::Get().GetFrontend()->ClosePort(port);

  if (port < m_ports.size())
    m_ports[port].Clear();
}
