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
#include "ButtonMapper.h"
#include "libretro/libretro.h"
#include "libretro/LibretroEnvironment.h"

#include "kodi/libKODI_game.h"

using namespace LIBRETRO;
using namespace P8PLATFORM;

#define LIBRETRO_JOYPAD_BUTTON_COUNT     16
#define LIBRETRO_ANALOG_STICK_COUNT      2
#define LIBRETRO_ACCELEROMETER_COUNT     1
#define LIBRETRO_MOUSE_BUTTON_COUNT      2
#define LIBRETRO_LIGHTGUN_BUTTON_COUNT   5
#define LIBRETRO_RELATIVE_POINTER_COUNT  1
#define LIBRETRO_ABSOLUTE_POINTER_COUNT  10

CLibretroDevice::CLibretroDevice(const game_controller* controller /* = NULL */)
  : m_type(RETRO_DEVICE_NONE)
{
  if (controller)
  {
    m_controllerId = controller->controller_id ? controller->controller_id : "";
    m_type = CButtonMapper::Get().GetLibretroType(m_controllerId);

    switch (m_type)
    {
      case RETRO_DEVICE_JOYPAD:
        m_buttons.resize(LIBRETRO_JOYPAD_BUTTON_COUNT);
        break;

      case RETRO_DEVICE_MOUSE:
        m_buttons.resize(LIBRETRO_MOUSE_BUTTON_COUNT);
        m_relativePointers.resize(LIBRETRO_RELATIVE_POINTER_COUNT);
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
  const std::string strControllerId = event.controller_id ? event.controller_id : "";
  const std::string strFeatureName = event.feature_name ? event.feature_name : "";

  int index = CButtonMapper::Get().GetLibretroIndex(strControllerId, strFeatureName);
  if (index >= 0)
  {
    switch (event.type)
    {
      case GAME_INPUT_EVENT_DIGITAL_BUTTON:
        if (index < (int)m_buttons.size())
          m_buttons[index] = event.digital_button;
        break;

      case GAME_INPUT_EVENT_ANALOG_BUTTON:
        // Not used by libretro
        break;

      case GAME_INPUT_EVENT_ANALOG_STICK:
        if (index < (int)m_analogSticks.size())
          m_analogSticks[index] = event.analog_stick;
        break;

      case GAME_INPUT_EVENT_ACCELEROMETER:
        if (index < (int)m_accelerometers.size())
          m_accelerometers[index] = event.accelerometer;
        break;

      case GAME_INPUT_EVENT_RELATIVE_POINTER:
        if (index < (int)m_relativePointers.size())
        {
          CLockObject lock(m_relativePtrMutex);

          m_relativePointers[index].x += event.rel_pointer.x;
          m_relativePointers[index].y += event.rel_pointer.y;
        }
        break;

      case GAME_INPUT_EVENT_ABSOLUTE_POINTER:
        if (index < (int)m_absolutePointers.size())
          m_absolutePointers[index] = event.abs_pointer;
        break;

      default:
        break;
    }

    return true;
  }

  return false;
}
