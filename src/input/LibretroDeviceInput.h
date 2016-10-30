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
#pragma once

#include "kodi_game_types.h"
#include "p8-platform/threads/mutex.h"

#include <string>
#include <vector>

namespace LIBRETRO
{
  typedef unsigned int libretro_device_t;

  class CLibretroDeviceInput
  {
  public:
    CLibretroDeviceInput(const game_controller* controller);

    bool  ButtonState(unsigned int buttonIndex) const;
    bool  AnalogStickState(unsigned int analogStickIndex, float& x, float& y) const;
    bool  AccelerometerState(float& x, float& y, float& z) const;
    int   RelativePointerDeltaX(void);
    int   RelativePointerDeltaY(void);
    bool  AbsolutePointerState(unsigned int pointerIndex, float& x, float& y) const;

    bool InputEvent(const game_input_event& event);

  private:
    std::vector<game_digital_button_event> m_buttons;
    std::vector<game_analog_stick_event>   m_analogSticks;
    std::vector<game_accelerometer_event>  m_accelerometers;
    std::vector<game_rel_pointer_event>    m_relativePointers;
    std::vector<game_abs_pointer_event>    m_absolutePointers;
    P8PLATFORM::CMutex                     m_relativePtrMutex;
  };
}
