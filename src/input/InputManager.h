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

#include "LibretroDevice.h"

#include "kodi_game_types.h"
#include "p8-platform/threads/mutex.h"

#include <stdint.h>
#include <string>
#include <vector>

struct retro_controller_info;
struct retro_input_descriptor;

namespace LIBRETRO
{
  typedef uint64_t  libretro_device_caps_t;

  class CInputManager
  {
  private:
    CInputManager(void) { }

  public:
    static CInputManager& Get(void);

    /*!
     * \brief Get the devices supported by the libretro wrapper
     *
     * \return A bitmask of libretro devices that this class supports
     */
    libretro_device_caps_t GetDeviceCaps(void) const;

    /*!
     * \brief Called when a device has been connected to an open port
     */
    void DeviceConnected(int port, bool bConnected, const game_controller* connectedController);

    /*!
     * \brief Get the libretro device abstraction for the device connected to
     *        the specified port
     */
    libretro_device_t GetDevice(unsigned int port);

    bool OpenPort(unsigned int port);
    DevicePtr GetPort(unsigned int port);
    void ClosePort(unsigned int port);
    void ClosePorts(void);

    /*!
     * \brief Enable or disable the port's analog sensors (enabled by default)
     *
     * \param port      The port
     * \param bEnabled  True to enable port's analog sensors, false to disable
     */
    void EnableAnalogSensors(unsigned int port, bool bEnabled);

    /*!
     * \brief Called when an input event has occurred
     */
    bool InputEvent(const game_input_event& event);

    /*!
     * \brief Parse libretro input descriptors and output to the log
     */
    void LogInputDescriptors(const retro_input_descriptor* descriptors);

    /*!
     * \brief Return the controller ID that the specified port is connected to
     */
    std::string ControllerID(unsigned int port);

    bool ButtonState(libretro_device_t device, unsigned int port, unsigned int buttonIndex);
    int DeltaX(libretro_device_t device, unsigned int port);
    int DeltaY(libretro_device_t device, unsigned int port);
    bool AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y);
    bool AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y);
    bool AccelerometerState(unsigned int port, float& x, float& y, float& z);

    /*!
     * \brief Inform the frontend of controller info
     */
    void SetControllerInfo(const retro_controller_info* info);

  private:
    void HandlePress(const game_key_event& key);
    bool IsPressed(uint32_t character);

    std::map<int, DevicePtr>          m_devices;
    std::vector<game_key_event>       m_pressedKeys;
    P8PLATFORM::CMutex                m_keyMutex;
  };
}
