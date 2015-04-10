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

#include "LibretroDevice.h"
#include "ClientBridge.h"
#include "LibretroEnvironment.h"
#include "LibretroTranslator.h"

#include "kodi/libKODI_game.h"

using namespace LIBRETRO;
using namespace PLATFORM;

#define LIBRETRO_JOYPAD_BUTTON_COUNT     16
#define LIBRETRO_ANALOG_STICK_COUNT      2
#define LIBRETRO_ACCELEROMETER_COUNT     1
#define LIBRETRO_MOUSE_BUTTON_COUNT      2
#define LIBRETRO_LIGHTGUN_BUTTON_COUNT   5
#define LIBRETRO_RELATIVE_POINTER_COUNT  1
#define LIBRETRO_ABSOLUTE_POINTER_COUNT  10

CLibretroDevice::CLibretroDevice(const game_input_device* device /* = NULL */)
  : m_type(RETRO_DEVICE_NONE)
{
  if (device)
  {
    m_type = GetFeature(device->device_id);

    switch (m_type)
    {
      case RETRO_DEVICE_JOYPAD:
        m_buttons.resize(LIBRETRO_JOYPAD_BUTTON_COUNT);
        break;

      case RETRO_DEVICE_MOUSE:
        m_buttons.resize(LIBRETRO_MOUSE_BUTTON_COUNT);
        m_relativePointers.resize(LIBRETRO_RELATIVE_POINTER_COUNT);
        break;

      case RETRO_DEVICE_KEYBOARD:
        // TODO
        break;

      case RETRO_DEVICE_LIGHTGUN:
        m_buttons.resize(LIBRETRO_LIGHTGUN_BUTTON_COUNT);
        m_relativePointers.resize(LIBRETRO_RELATIVE_POINTER_COUNT);
        break;

      case RETRO_DEVICE_ANALOG:
        m_buttons.resize(LIBRETRO_JOYPAD_BUTTON_COUNT);
        m_analogSticks.resize(LIBRETRO_ANALOG_STICK_COUNT);
        break;

      case RETRO_DEVICE_POINTER:
        m_absolutePointers.resize(LIBRETRO_ABSOLUTE_POINTER_COUNT);
        break;

      default:
        break;
    }

    m_accelerometers.resize(LIBRETRO_ACCELEROMETER_COUNT);
  }
}

CLibretroDevice& CLibretroDevice::operator=(const CLibretroDevice& rhs)
{
  if (this != &rhs)
  {
    m_type             = rhs.m_type;
    m_buttons          = rhs.m_buttons;
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
  m_buttons.clear();
  m_analogSticks.clear();
  m_accelerometers.clear();
  m_relativePointers.clear();
  m_absolutePointers.clear();
}

bool CLibretroDevice::ButtonState(unsigned int buttonIndex) const
{
  bool buttonState = false;

  if (buttonIndex < m_buttons.size())
    buttonState = m_buttons[buttonIndex].pressed;

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
        if (index < m_buttons.size())
          m_buttons[index] = event.digital_button;
        break;

      case GAME_INPUT_EVENT_ANALOG_BUTTON:
        // Not used by libretro
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
        // libretro keyboard is event-based
        if (CLibretroEnvironment::Get().GetClient())
        {
          /* TODO
          CLibretroEnvironment::Get().GetClient()->KeyboardEvent(event.key.pressed,
                                                                 0, // TODO
                                                                 event.key.character,
                                                                 LibretroTranslator::GetKeyModifiers(event.key.modifiers));
          */
        }
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

libretro_device_t CLibretroDevice::GetFeature(const std::string& strDeviceId) const
{
  if (strDeviceId == "game.controller.default")
    return RETRO_DEVICE_ANALOG;
  else if (strDeviceId == "game.controller.nes")
    return RETRO_DEVICE_JOYPAD;
  else if (strDeviceId == "game.controller.snes")
    return RETRO_DEVICE_JOYPAD;

  return RETRO_DEVICE_NONE;
}
