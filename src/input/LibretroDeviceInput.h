/*
 *  Copyright (C) 2015-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/addon-instance/Game.h>
#include <mutex>

#include <string>
#include <vector>

namespace LIBRETRO
{
  typedef unsigned int libretro_device_t;

  class CLibretroDeviceInput
  {
  public:
    CLibretroDeviceInput(const std::string &controllerId);

    bool  ButtonState(unsigned int buttonIndex) const;
    float AnalogButtonState(unsigned int buttonIndex) const;
    bool  AnalogStickState(unsigned int analogStickIndex, float& x, float& y) const;
    bool  AccelerometerState(float& x, float& y, float& z) const;
    int   RelativePointerDeltaX(void);
    int   RelativePointerDeltaY(void);
    bool  AbsolutePointerState(unsigned int pointerIndex, float& x, float& y) const;

    bool InputEvent(const game_input_event& event);

  private:
    /*!
     * \brief Report key to client
     */
    void SendKeyEvent(const std::string &controllerId,
                      const std::string &feature,
                      unsigned int keyIndex,
                      const game_key_event &keyEvent);

    std::vector<game_digital_button_event> m_buttons;
    std::vector<game_analog_button_event>  m_analogButtons;
    std::vector<game_analog_stick_event>   m_analogSticks;
    std::vector<game_accelerometer_event>  m_accelerometers;
    std::vector<game_rel_pointer_event>    m_relativePointers;
    std::vector<game_abs_pointer_event>    m_absolutePointers;
    std::mutex                             m_relativePtrMutex;
  };
}
