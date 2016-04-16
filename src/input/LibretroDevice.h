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
#pragma once

#include "kodi/kodi_game_types.h"
#include "p8-platform/threads/mutex.h"

#include <string>
#include <vector>

namespace LIBRETRO
{
  typedef unsigned int libretro_device_t;

  class CLibretroDevice
  {
  public:
    CLibretroDevice(const game_controller* controller = NULL);
    CLibretroDevice(const CLibretroDevice& other) { *this = other; }

    CLibretroDevice& operator=(const CLibretroDevice& rhs);

    libretro_device_t Type(void) const { return m_type; }
    std::string ControllerID(void) const { return m_controllerId; }

    void Clear(void);

    bool  ButtonState(unsigned int buttonIndex) const;
    bool  AnalogStickState(unsigned int analogStickIndex, float& x, float& y) const;
    bool  AccelerometerState(float& x, float& y, float& z) const;
    int   RelativePointerDeltaX(void);
    int   RelativePointerDeltaY(void);
    bool  AbsolutePointerState(unsigned int pointerIndex, float& x, float& y) const;

    bool InputEvent(const game_input_event& event);

  private:
    libretro_device_t                      m_type;
    std::string                            m_controllerId;
    std::vector<game_digital_button_event> m_buttons;
    std::vector<game_analog_stick_event>   m_analogSticks;
    std::vector<game_accelerometer_event>  m_accelerometers;
    std::vector<game_rel_pointer_event>    m_relativePointers;
    std::vector<game_abs_pointer_event>    m_absolutePointers;
    P8PLATFORM::CMutex                       m_relativePtrMutex;
  };
}
