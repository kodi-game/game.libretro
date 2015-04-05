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
#include "kodi/threads/mutex.h"

#include <stdint.h>
#include <string>
#include <vector>

struct retro_input_descriptor;

namespace LIBRETRO
{
  typedef unsigned int libretro_device_t;
  typedef uint64_t     libretro_device_caps_t;

  class CLibretroDevice
  {
  public:
    CLibretroDevice(const game_input_device* device = NULL);
    CLibretroDevice(const CLibretroDevice& other) { *this = other; }

    CLibretroDevice& operator=(const CLibretroDevice& rhs);

    libretro_device_t Type(void) const { return m_type; }

    void Clear(void);

    bool DigitalButtonState(unsigned int buttonIndex) const;
    bool AnalogStickState(unsigned int analogStickIndex, float& x, float& y) const;
    bool AccelerometerState(float& x, float& y, float& z) const;
    int RelativePointerDeltaX(void);
    int RelativePointerDeltaY(void);
    bool AbsolutePointerState(unsigned int pointerIndex, float& x, float& y) const;

    bool InputEvent(const game_input_event& event);

  private:
    int GetLibretroIndex(const std::string& strDeviceId, const std::string& strFeatureName) const;

    libretro_device_t                      m_type;
    std::vector<game_digital_button_event> m_digitalButtons;
    std::vector<game_analog_stick_event>   m_analogSticks;
    std::vector<game_accelerometer_event>  m_accelerometers;
    std::vector<game_rel_pointer_event>    m_relativePointers;
    std::vector<game_abs_pointer_event>    m_absolutePointers;
    PLATFORM::CMutex                       m_relativePtrMutex;
  };

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
    void DeviceConnected(unsigned int port, bool bConnected, const game_input_device* connectedDevice);

    /*!
     * \brief Get the libretro device abstraction for the device connected to
     *        the specified port
     */
    libretro_device_t GetDevice(unsigned int port) const;

    bool OpenPort(unsigned int port);
    void ClosePort(unsigned int port);
    void ClosePorts(void);

    /*!
     * \brief Enable input events for the specified source
     */
    void EnableSource(bool bEnabled, unsigned int port, GAME_INPUT_EVENT_SOURCE source, unsigned int index);

    /*!
     * \brief Called when an input event has occurred
     */
    bool InputEvent(unsigned int port, const game_input_event& event);

    /*!
     * \brief Parse libretro input descriptors and output to the log
     */
    void LogInputDescriptors(const retro_input_descriptor* descriptors);

    bool DigitalButtonState(libretro_device_t device, unsigned int port, unsigned int buttonIndex);
    int DeltaX(libretro_device_t device, unsigned int port);
    int DeltaY(libretro_device_t device, unsigned int port);
    bool AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y);
    bool AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y);
    bool AccelerometerState(unsigned int port, float& x, float& y, float& z);

  private:
    std::vector<CLibretroDevice> m_ports;
  };
}
