/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "InputTypes.h"
#include "ControllerLayout.h"
#include "LibretroDevice.h"

#include <kodi/addon-instance/Game.h>
#include <mutex>

#include <map>
#include <memory>
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
     * \brief
     */
    void SetControllerLayouts(const std::vector<kodi::addon::GameControllerLayout>& controllers);

    /*!
     * \brief Enable the keyboard
     *
     * \param controllerId The keyboard's controller ID
     *
     * \return True if keyboard was enabled, false otherwise
     */
    bool EnableKeyboard(const std::string &controller);

    /*!
     * \brief Disable the keyboard and free any resources it held
     */
    void DisableKeyboard();

    /*!
     * \brief Enable mouse input
     *
     * \param controllerId The mouse's controller ID
     *
     * \return True if mouse input was enabled, false otherwise
     */
    bool EnableMouse(const std::string &controllerId);

    /*!
     * \brief Disable the mouse and free any resources it held
     */
    void DisableMouse();

    /*!
     * \brief Called when a device has been connected to a port
     */
    libretro_device_t ConnectController(const std::string &address, const std::string &controllerId);

    /*!
     * \brief Called when a device has been disconnected from a port
     */
    bool DisconnectController(const std::string &address);

    /*!
     * \brief Get the port number associated with the specified address
     *
     * \return The port number, or -1 if the address has no port number
     */
    int GetPortIndex(const std::string &address) const;

    /*!
     * \brief Get the port number that we should use when setting the
     * controller port
     *
     * \param address The port address
     * \param[out] connectionPort The port used to change connections, or
     * untouched if this function returns false
     *
     * \return True if a connection port was specified, false otherwise
     */
    bool GetConnectionPortIndex(const std::string &address, int& connectionPort) const;

    std::string GetAddress(unsigned int port) const;

    /*!
     * \brief Get the libretro device abstraction for the device connected to
     *        the specified address
     */
    libretro_device_t GetDeviceType(const std::string &address) const;

    /*!
     * \brief Close all ports
     */
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
    void LogInputDescriptors(const retro_input_descriptor* descriptors) const;

    /*!
     * \brief Return the controller ID that the specified port is connected to
     */
    std::string ControllerID(unsigned int port) const;

    bool ButtonState(libretro_device_t device, unsigned int port, unsigned int buttonIndex) const;
    float AnalogButtonState(unsigned int port, unsigned int buttonIndex) const;
    int DeltaX(libretro_device_t device, unsigned int port);
    int DeltaY(libretro_device_t device, unsigned int port);
    bool AnalogStickState(unsigned int port, unsigned int analogStickIndex, float& x, float& y) const;
    bool AbsolutePointerState(unsigned int port, unsigned int pointerIndex, float& x, float& y) const;
    bool AccelerometerState(unsigned int port, float& x, float& y, float& z) const;

    /*!
     * \brief Inform the frontend of controller info
     */
    void SetControllerInfo(const retro_controller_info* info);

  private:
    DevicePtr m_keyboard;
    DevicePtr m_mouse;
    DeviceVector m_controllers;
    std::map<std::string, std::unique_ptr<CControllerLayout>> m_controllerLayouts;
  };
}
