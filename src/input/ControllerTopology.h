/*
 *      Copyright (C) 2017 Team Kodi
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

    std::string GetAddress(unsigned int portIndex) const;

    bool SetDevice(GAME_PORT_TYPE portType, const std::string &controllerId);
    void RemoveDevice(GAME_PORT_TYPE portType);

    bool SetController(const std::string &portAddress, const std::string &controllerId, bool bProvidesInput);
    void RemoveController(const std::string &portAddress);

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
      std::vector<ControllerPtr> accepts;
      std::string activeId; // Empty if disconnected
    };

    struct Controller
    {
      std::string controllerId;
      std::vector<PortPtr> ports;
      bool bProvidesInput;
    };

    static int GetPortIndex(const PortPtr &port, const std::string &portAddress, unsigned int &playerCount);
    static int GetPortIndex(const ControllerPtr &controller, const std::string &portAddress, unsigned int &playerCount);

    static std::string GetAddress(const PortPtr &port, unsigned int portIndex, unsigned int &playerCount);
    static std::string GetAddress(const ControllerPtr &controller, unsigned int portIndex, unsigned int &playerCount);

    static bool SetController(const PortPtr &port, const std::string &portAddress, const std::string &controllerId, bool bProvidesInput);
    static void RemoveController(const PortPtr &port, const std::string &portAddress);

    static bool SetController(const ControllerPtr &controller, const std::string &portAddress, const std::string &controllerId, bool bProvidesInput);
    static void RemoveController(const ControllerPtr &controller, const std::string &portAddress);

    static PortPtr CreateDefaultPort(const std::string &acceptedController);

    static void SplitAddress(const std::string &address, std::string &nodeId, std::string &remainingAddress);

    std::vector<PortPtr> m_ports;
    int m_playerLimit = -1;
  };
}
