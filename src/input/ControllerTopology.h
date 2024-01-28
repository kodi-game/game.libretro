/*
 *  Copyright (C) 2017-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "input/InputTypes.h"

#include <kodi/addon-instance/Game.h>
#include <libretro-common/libretro.h>

#include <memory>
#include <string>
#include <vector>

class TiXmlElement;

namespace LIBRETRO
{
  class CControllerTopology
  {
  public:
    CControllerTopology() = default;

    static CControllerTopology& GetInstance();

    bool LoadTopology();

    void Clear();

    game_input_topology* GetTopology();

    static void FreeTopology(game_input_topology *topology);

    static void FreePorts(game_input_port *ports, unsigned int portCount);

    int GetPortIndex(const std::string &address) const;

    bool GetConnectionPortIndex(const std::string &address, int &connectionPortId) const;

    std::string GetAddress(unsigned int portIndex) const;

    bool SetDevice(GAME_PORT_TYPE portType, const std::string &controllerId);
    void RemoveDevice(GAME_PORT_TYPE portType);

    bool SetController(const std::string &portAddress, const std::string &controllerId, bool bProvidesInput);
    void RemoveController(const std::string &portAddress);

    libretro_device_t TypeOverride(const std::string &portAddress, const std::string &controllerId) const;
    libretro_subclass_t SubclassOverride(const std::string &portAddress, const std::string &controllerId) const;

    int PlayerLimit() const { return m_playerLimit; }

  private:
    struct Port;
    using PortPtr = std::unique_ptr<Port>;

    struct Controller;
    using ControllerPtr = std::unique_ptr<Controller>;

    bool Deserialize(const TiXmlElement* pElement);
    PortPtr DeserializePort(const TiXmlElement* pElement);
    ControllerPtr DeserializeController(const TiXmlElement* pElement);

    static game_input_port *GetPorts(const std::vector<PortPtr> &portVec, unsigned int &portCount);
    static game_input_device *GetControllers(const std::vector<ControllerPtr> &controllerVec, unsigned int &deviceCount);
    static void FreeControllers(game_input_device *devices, unsigned int deviceCount);

    struct Port
    {
      GAME_PORT_TYPE type;
      std::string portId;
      std::string connectionPort; // Empty if no connection port is specified in topology.xml
      bool forceConnected = false;
      std::vector<ControllerPtr> accepts;
      std::string activeId; // Empty if disconnected
    };

    struct Controller
    {
      std::string controllerId;
      std::vector<PortPtr> ports;
      bool bProvidesInput;
      libretro_device_t type = RETRO_DEVICE_NONE;
      libretro_subclass_t subclass = RETRO_SUBCLASS_NONE;
    };

    static unsigned int GetPlayerCount(const PortPtr& port);
    static unsigned int GetPlayerCount(const ControllerPtr& controller);

    static int GetPortIndex(const PortPtr &port, const std::string &portAddress, unsigned int &playerCount);
    static int GetPortIndex(const ControllerPtr &controller, const std::string &portAddress, unsigned int &playerCount);

    static bool GetConnectionPortIndex(const PortPtr &port, const std::string &portAddress, int &connectionPort);
    static bool GetConnectionPortIndex(const ControllerPtr &controller, const std::string &portAddress, int &connectionPort);

    static std::string GetAddress(const PortPtr &port, unsigned int portIndex, unsigned int &playerCount);
    static std::string GetAddress(const ControllerPtr &controller, unsigned int portIndex, unsigned int &playerCount);

    static bool SetController(const PortPtr &port, const std::string &portAddress, const std::string &controllerId, bool bProvidesInput);
    static void RemoveController(const PortPtr &port, const std::string &portAddress);

    static bool SetController(const ControllerPtr &controller, const std::string &portAddress, const std::string &controllerId, bool bProvidesInput);
    static void RemoveController(const ControllerPtr &controller, const std::string &portAddress);

    static libretro_device_t TypeOverride(const std::vector<PortPtr>& ports, const std::string& controllerAddress);
    static libretro_subclass_t SubclassOverride(const std::vector<PortPtr>& ports, const std::string& controllerAddress);

    static libretro_device_t TypeOverride(const std::vector<ControllerPtr>& controllers, const std::string& controllerAddress);
    static libretro_subclass_t SubclassOverride(const std::vector<ControllerPtr>& controllers, const std::string &controllerAddress);

    static PortPtr CreateDefaultPort(const std::string &acceptedController);

    static const ControllerPtr& GetActiveController(const PortPtr& port);

    static void SplitAddress(const std::string &address, std::string &nodeId, std::string &remainingAddress);
    static std::string JoinAddress(const std::string& address, const std::string& nodeId);

    std::vector<PortPtr> m_ports;
    int m_playerLimit = -1;
  };
}
