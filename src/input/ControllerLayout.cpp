/*
 *      Copyright (C) 2018 Team Kodi
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

#include "ControllerLayout.h"

#include "kodi_game_types.h"

using namespace LIBRETRO;

CControllerLayout::CControllerLayout(const game_controller_layout &controller) :
  m_controllerId(controller.controller_id != nullptr ? controller.controller_id : ""),
  m_bProvidesInput(controller.provides_input)
{
  if (controller.digital_buttons != nullptr)
  {
    for (unsigned int i = 0; i < controller.digital_button_count; i++)
      m_digitalButtons.emplace_back(controller.digital_buttons[i]);
  }

  if (controller.analog_buttons != nullptr)
  {
    for (unsigned int i = 0; i < controller.analog_button_count; i++)
      m_analogButtons.emplace_back(controller.analog_buttons[i]);
  }

  if (controller.analog_sticks != nullptr)
  {
    for (unsigned int i = 0; i < controller.analog_stick_count; i++)
      m_analogSticks.emplace_back(controller.analog_sticks[i]);
  }

  if (controller.accelerometers != nullptr)
  {
    for (unsigned int i = 0; i < controller.accelerometer_count; i++)
      m_accelerometers.emplace_back(controller.accelerometers[i]);
  }

  if (controller.keys != nullptr)
  {
    for (unsigned int i = 0; i < controller.key_count; i++)
      m_keys.emplace_back(controller.keys[i]);
  }

  if (controller.rel_pointers != nullptr)
  {
    for (unsigned int i = 0; i < controller.rel_pointer_count; i++)
      m_relPointers.emplace_back(controller.rel_pointers[i]);
  }

  if (controller.abs_pointers != nullptr)
  {
    for (unsigned int i = 0; i < controller.abs_pointer_count; i++)
      m_absPointers.emplace_back(controller.abs_pointers[i]);
  }

  if (controller.motors != nullptr)
  {
    for (unsigned int i = 0; i < controller.motor_count; i++)
      m_motors.emplace_back(controller.motors[i]);
  }
}
