/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroDeviceInput.h"
#include "LibretroDevice.h"
#include "ButtonMapper.h"
#include "libretro/ClientBridge.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/LibretroTranslator.h"
#include "libretro/libretro.h"
#include "log/Log.h"

using namespace LIBRETRO;

#define LIBRETRO_JOYPAD_BUTTON_COUNT     16
#define LIBRETRO_ANALOG_STICK_COUNT      2
#define LIBRETRO_ACCELEROMETER_COUNT     1
#define LIBRETRO_MOUSE_BUTTON_COUNT      11
#define LIBRETRO_LIGHTGUN_BUTTON_COUNT   17
#define LIBRETRO_RELATIVE_POINTER_COUNT  1
#define LIBRETRO_ABSOLUTE_POINTER_COUNT  10

#define ANALOG_DIGITAL_THRESHHOLD  0.5f

CLibretroDeviceInput::CLibretroDeviceInput(const std::string &controllerId)
{
  unsigned int type = CButtonMapper::Get().GetLibretroType(controllerId);

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

    case RETRO_DEVICE_KEYBOARD:
      m_buttons.resize(RETROK_LAST - 1);
      break;

    default:
      break;
  }

  m_accelerometers.resize(LIBRETRO_ACCELEROMETER_COUNT);
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
    std::unique_lock<std::mutex> lock(m_relativePtrMutex);

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
    std::unique_lock<std::mutex> lock(m_relativePtrMutex);

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

      case GAME_INPUT_EVENT_KEY:
      {
        // Send keypress to libertro
        SendKeyEvent(strControllerId, strFeatureName, index, event.key);

        // Save keypress for polling
        if (static_cast<size_t>(index) < m_buttons.size())
          m_buttons[index] = event.digital_button;

        break;
      }

      case GAME_INPUT_EVENT_RELATIVE_POINTER:
        if (index < (int)m_relativePointers.size())
        {
          std::unique_lock<std::mutex> lock(m_relativePtrMutex);

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

void CLibretroDeviceInput::SendKeyEvent(const std::string &controllerId,
                                        const std::string &feature,
                                        unsigned int keyIndex,
                                        const game_key_event &keyEvent)
{
  // Report key to client
  CClientBridge* clientBridge = CLibretroEnvironment::Get().GetClientBridge();
  if (clientBridge)
  {
    const bool down = keyEvent.pressed;
    const retro_key keycode = static_cast<retro_key>(keyIndex);
    const uint32_t character = keyEvent.unicode;
    const retro_mod key_modifiers = LibretroTranslator::GetKeyModifiers(keyEvent.modifiers);
    std::string retroKey = LibretroTranslator::GetFeatureName(RETRO_DEVICE_KEYBOARD, 0, keycode);

    dsyslog("Controller \"%s\" key \"%s\" (%s) modifier 0x%08x: %s",
        controllerId.c_str(),
        feature.c_str(),
        retroKey.c_str(),
        keyEvent.modifiers,
        down ? "down" : "up");

    clientBridge->KeyboardEvent(down, keycode, character, key_modifiers);
  }
}
