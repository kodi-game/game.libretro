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

#include "LibretroDeviceInput.h"
#include "LibretroDevice.h"
#include "ButtonMapper.h"
#include "libretro/libretro.h"

using namespace LIBRETRO;
using namespace P8PLATFORM;

#define LIBRETRO_JOYPAD_BUTTON_COUNT     16
#define LIBRETRO_ANALOG_STICK_COUNT      2
#define LIBRETRO_ACCELEROMETER_COUNT     1
#define LIBRETRO_MOUSE_BUTTON_COUNT      11
#define LIBRETRO_LIGHTGUN_BUTTON_COUNT   17
#define LIBRETRO_RELATIVE_POINTER_COUNT  1
#define LIBRETRO_ABSOLUTE_POINTER_COUNT  10

#define ANALOG_DIGITAL_THRESHHOLD  0.5f

CLibretroDeviceInput::CLibretroDeviceInput(const game_controller &controller)
{
  if (controller.controller_id != nullptr)
  {
    unsigned int type = CButtonMapper::Get().GetLibretroType(controller.controller_id);

    switch (type)
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
        m_analogButtons.resize(LIBRETRO_JOYPAD_BUTTON_COUNT);
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

bool CLibretroDeviceInput::ButtonState(unsigned int buttonIndex) const
{
  bool buttonState = false;

  if (buttonIndex < m_buttons.size())
    buttonState = m_buttons[buttonIndex].pressed;

  return buttonState;
}

float CLibretroDeviceInput::AnalogButtonState(unsigned int buttonIndex) const
{
  float axisState = 0.0f;

  if (buttonIndex < m_analogButtons.size())
    axisState = m_analogButtons[buttonIndex].magnitude;

  return axisState;
}

bool CLibretroDeviceInput::AnalogStickState(unsigned int analogStickIndex, float& x, float& y) const
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

bool CLibretroDeviceInput::AccelerometerState(float& x, float& y, float& z) const
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

int CLibretroDeviceInput::RelativePointerDeltaX(void)
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

int CLibretroDeviceInput::RelativePointerDeltaY(void)
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

bool CLibretroDeviceInput::AbsolutePointerState(unsigned int pointerIndex, float& x, float& y) const
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

bool CLibretroDeviceInput::InputEvent(const game_input_event& event)
{
  const std::string strControllerId = event.controller_id ? event.controller_id : "";
  const std::string strFeatureName = event.feature_name ? event.feature_name : "";

  int index = CButtonMapper::Get().GetLibretroIndex(strControllerId, strFeatureName);
  if (index >= 0)
  {
    switch (event.type)
    {
      case GAME_INPUT_EVENT_DIGITAL_BUTTON:
      {
        if (index < (int)m_buttons.size())
          m_buttons[index] = event.digital_button;

        if (index < static_cast<int>(m_analogButtons.size()))
          m_analogButtons[index].magnitude = event.digital_button.pressed ? 1.0f : 0.0f;

        break;
      }

      case GAME_INPUT_EVENT_ANALOG_BUTTON:
      {
        if (index < static_cast<int>(m_buttons.size()))
          m_buttons[index].pressed = (event.analog_button.magnitude >= ANALOG_DIGITAL_THRESHHOLD);

        if (index < static_cast<int>(m_analogButtons.size()))
          m_analogButtons[index].magnitude = event.analog_button.magnitude;

        break;
      }

      case GAME_INPUT_EVENT_AXIS:
      {
        const int axisId = CButtonMapper::Get().GetAxisID(strControllerId, strFeatureName);
        if (axisId >= 0)
        {
          const libretro_device_t deviceType = CButtonMapper::Get().GetLibretroDevice(strControllerId, strFeatureName);
          switch (deviceType)
          {
          case RETRO_DEVICE_ANALOG:
          {
            if (index < static_cast<int>(m_analogSticks.size()))
            {
              auto& analogStick = m_analogSticks[index];
              switch (axisId)
              {
              case RETRO_DEVICE_ID_ANALOG_X:
                analogStick.x = event.axis.position;
                break;
              case RETRO_DEVICE_ID_ANALOG_Y:
                analogStick.y = event.axis.position;
                break;
              default:
                break;
              }
            }
            break;
          }
          case RETRO_DEVICE_POINTER:
          {
            if (index < static_cast<int>(m_absolutePointers.size()))
            {
              auto& absPointer = m_absolutePointers[index];
              switch (axisId)
              {
              case RETRO_DEVICE_ID_POINTER_X:
                absPointer.x = event.axis.position;
                break;
              case RETRO_DEVICE_ID_POINTER_Y:
                absPointer.y = event.axis.position;
                break;
              default:
                break;
              }
            }
            break;
          }
          default:
            break;
          }
        }

        break;
      }

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
