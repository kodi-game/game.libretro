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

using namespace LIBRETRO;

CInputManager& CInputManager::Get(void)
{
  static CInputManager _instance;
  return _instance;
}

libretro_device_caps_t CInputManager::GetDeviceCaps(void)
{
  return (1 << RETRO_DEVICE_JOYPAD); // TODO
}

libretro_device_t CInputManager::UpdatePort(unsigned int port, bool bPortConnected)
{
  return RETRO_DEVICE_JOYPAD; // TODO
}

void CInputManager::EnableSource(bool bEnabled, unsigned int port, GAME_INPUT_EVENT_SOURCE source, unsigned int index)
{
  // TODO
}

void CInputManager::InputEvent(unsigned int port, const game_input_event& event)
{
  // TODO
}

void CInputManager::SetInputDescriptors(const retro_input_descriptor* descriptors)
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

bool CInputManager::DigitalButtonState(unsigned int port, unsigned int index)
{
  return false; // TODO
}

bool CInputManager::AnalogStickState(unsigned int port, unsigned int index, float& x, float& y)
{
  x = 0.0f;
  y = 0.0f;
  return true; // TODO
}

bool CInputManager::AccelerometerState(unsigned int port, unsigned int index, float& x, float& y, float& z)
{
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
  return true; // TODO
}

int16_t CInputManager::MouseDeltaX(unsigned int port)
{
  return 0; // TODO
}

int16_t CInputManager::MouseDeltaY(unsigned int port)
{
  return 0; // TODO
}
