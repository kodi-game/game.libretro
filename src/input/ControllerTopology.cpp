/*
 *  Copyright (C) 2017-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "ControllerTopology.h"
#include "InputDefinitions.h"
#include "InputTranslator.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/LibretroTranslator.h"
#include "log/Log.h"

#include <tinyxml.h>

#include <algorithm>
#include <sstream>

using namespace LIBRETRO;

#define TOPOLOGY_XML          "topology.xml"

#define ADDRESS_SEPARATOR  '/'

//! @todo Remove me when these are added to the Game API
#if !defined(KEYBOARD_PORT_ID)
#define KEYBOARD_PORT_ID "keyboard"
#endif
#if !defined(MOUSE_PORT_ID)
#define MOUSE_PORT_ID "mouse"
#endif

CControllerTopology& CControllerTopology::GetInstance()
{
  static CControllerTopology instance;
  return instance;
}

bool CControllerTopology::LoadTopology()
{
  bool bSuccess = false;

  Clear();

  std::string strFilename = CLibretroEnvironment::Get().GetResourcePath(TOPOLOGY_XML);
  if (strFilename.empty())
  {
    dsyslog("Could not locate controller topology \"%s\"", TOPOLOGY_XML);
  }
  else
  {
    dsyslog("Loading controller topology \"%s\"", strFilename.c_str());

    TiXmlDocument topologyXml;
    if (topologyXml.LoadFile(strFilename))
    {
      TiXmlElement* pRootElement = topologyXml.RootElement();
      bSuccess = Deserialize(pRootElement);
    }
    else
    {
      esyslog("Failed to load controller topology: %s (line %d)", topologyXml.ErrorDesc(), topologyXml.ErrorRow());
    }
  }

  return bSuccess;
}

void CControllerTopology::Clear()
{
  m_ports.clear();
}

game_input_topology *CControllerTopology::GetTopology()
{
  if (!m_ports.empty())
  {
    game_input_topology *topology = new game_input_topology;

    int unsigned portCount = 0;
    topology->ports = GetPorts(m_ports, portCount);
    topology->port_count = portCount;
    topology->player_limit = m_playerLimit;

    return topology;
  }

  return nullptr;
}

void CControllerTopology::FreeTopology(game_input_topology *topology)
{
  if (topology != nullptr)
    FreePorts(topology->ports, topology->port_count);

  delete topology;
}

void CControllerTopology::FreePorts(game_input_port *ports, unsigned int portCount)
{
  if (ports != nullptr)
  {
    for (unsigned int i = 0; i < portCount; i++)
      FreeControllers(ports[i].accepted_devices, ports[i].device_count);
  }

  delete[] ports;
}

unsigned int CControllerTopology::GetPlayerCount(const PortPtr& port)
{
  unsigned int playerCount = 0;

  const ControllerPtr& controller = GetActiveController(port);
  if (controller)
    playerCount += GetPlayerCount(controller);

  return playerCount;
}

unsigned int CControllerTopology::GetPlayerCount(const ControllerPtr& controller)
{
  unsigned int playerCount = 0;

  if (controller->bProvidesInput)
    playerCount++;

  for (const PortPtr &port : controller->ports)
    playerCount += GetPlayerCount(port);

  return playerCount;
}

int CControllerTopology::GetPortIndex(const std::string &address) const
{
  int portIndex = -1;
  unsigned int playerCount = 0;

  if (m_ports.empty())
  {
    // If topology is unknown, use the first port
    portIndex = 0;
  }
  else
  {
    for (const auto &port : m_ports)
    {
      if (port->type == GAME_PORT_CONTROLLER)
      {
        portIndex = GetPortIndex(port, address, playerCount);
        if (portIndex >= 0)
          break;
      }
    }
  }

  // Reset port index if it exceeds the player limit
  if (m_playerLimit >= 0 && portIndex >= m_playerLimit)
    portIndex = -1;

  return portIndex;
}

int CControllerTopology::GetPortIndex(const PortPtr &port, const std::string &portAddress, unsigned int &playerCount)
{
  int portIndex = -1;

  std::string portId;
  std::string remainingAddress;
  SplitAddress(portAddress, portId, remainingAddress);

  if (port->portId == portId)
  {
    if (remainingAddress.empty())
    {
      // Base case
      portIndex = playerCount;
    }
    else
    {
      const ControllerPtr& controller = GetActiveController(port);
      if (controller)
      {
        // Player count is incremented as we visit controllers recursively
        portIndex = GetPortIndex(controller, remainingAddress, playerCount);
      }
    }
  }
  else
  {
    // Player count wasn't incremented by visiting controllers for this port,
    // so calculate and increment it now
    playerCount += GetPlayerCount(port);
  }

  return portIndex;
}

int CControllerTopology::GetPortIndex(const ControllerPtr &controller, const std::string &portAddress, unsigned int &playerCount)
{
  int portIndex = -1;

  if (controller->bProvidesInput)
    playerCount++;

  std::string portControllerId;
  std::string remainingAddress;
  SplitAddress(portAddress, portControllerId, remainingAddress);

  if (controller->controllerId == portControllerId)
  {
    const auto &ports = controller->ports;

    for (const auto &port : ports)
    {
      portIndex = GetPortIndex(port, remainingAddress, playerCount);
      if (portIndex >= 0)
        break;
    }
  }

  return portIndex;
}

bool CControllerTopology::GetConnectionPortIndex(const std::string &address, int &connectionPort) const
{
  for (const auto &port : m_ports)
  {
    if (GetConnectionPortIndex(port, address, connectionPort))
      return true;
  }

  return false;
}

bool CControllerTopology::GetConnectionPortIndex(const PortPtr &port, const std::string &portAddress, int &connectionPort)
{
  std::string portId;
  std::string remainingAddress;
  SplitAddress(portAddress, portId, remainingAddress);

  if (port->portId == portId)
  {
    if (remainingAddress.empty())
    {
      // Base case
      if (!port->connectionPort.empty())
      {
        std::istringstream(port->connectionPort) >> connectionPort;
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      const ControllerPtr& controller = GetActiveController(port);
      if (controller)
      {
        if (GetConnectionPortIndex(controller, remainingAddress, connectionPort))
          return true;
      }
    }
  }

  return false;
}

bool CControllerTopology::GetConnectionPortIndex(const ControllerPtr &controller, const std::string &portAddress, int &connectionPort)
{
  std::string portControllerId;
  std::string remainingAddress;
  SplitAddress(portAddress, portControllerId, remainingAddress);

  if (controller->controllerId == portControllerId)
  {
    const auto &ports = controller->ports;

    for (const auto &childPort : ports)
    {
      if (GetConnectionPortIndex(childPort, remainingAddress, connectionPort))
        return true;
    }
  }

  return false;
}

std::string CControllerTopology::GetAddress(unsigned int portIndex) const
{
  std::string address;
  unsigned int playerCount = 0;

  if (m_ports.empty())
  {
    return DEFAULT_PORT_ID;
  }
  else
  {
    for (const auto &port : m_ports)
    {
      if (port->type == GAME_PORT_CONTROLLER)
      {
        address = GetAddress(port, portIndex, playerCount);
        if (!address.empty())
          break;
      }
    }
  }

  return address;
}

std::string CControllerTopology::GetAddress(const PortPtr &port, unsigned int portIndex, unsigned int &playerCount)
{
  std::string address;

  if (portIndex == playerCount)
  {
    // Base case
    address = ADDRESS_SEPARATOR + port->portId;
  }
  else
  {

    const ControllerPtr& controller = GetActiveController(port);
    if (controller)
    {
      std::string controllerAddress = GetAddress(controller, portIndex, playerCount);
      if (!controllerAddress.empty())
        address = ADDRESS_SEPARATOR + port->portId + controllerAddress;
    }
  }

  playerCount++;

  return address;
}

std::string CControllerTopology::GetAddress(const ControllerPtr &controller, unsigned int portIndex, unsigned int &playerCount)
{
  std::string address;

  const auto &ports = controller->ports;
  for (const auto &port : ports)
  {
    std::string portAddress = GetAddress(port, portIndex, playerCount);
    if (!portAddress.empty())
    {
      address = ADDRESS_SEPARATOR + controller->controllerId + portAddress;
      break;
    }
  }

  if (controller->bProvidesInput)
    playerCount++;

  return address;
}

bool CControllerTopology::SetDevice(GAME_PORT_TYPE portType, const std::string &controllerId)
{
  for (const auto &port : m_ports)
  {
    if (port->type == portType)
    {
      const auto &accepts = port->accepts;

      auto it = std::find_if(accepts.begin(), accepts.end(),
        [&controllerId](const ControllerPtr &controller)
        {
          return controllerId == controller->controllerId;
        });

      if (it != accepts.end())
      {
        port->activeId = controllerId;
        return true;
      }
    }
  }

  return false;
}

void CControllerTopology::RemoveDevice(GAME_PORT_TYPE portType)
{
  for (const auto &port : m_ports)
  {
    if (port->type == portType)
      port->activeId.clear();
  }
}

bool CControllerTopology::SetController(const std::string &portAddress, const std::string &controllerId, bool bProvidesInput)
{
  if (m_ports.empty())
  {
    // No topology was specified, create one now
    m_ports.emplace_back(CreateDefaultPort(controllerId));
  }

  for (const auto &port : m_ports)
  {
    if (port->type == GAME_PORT_CONTROLLER)
    {
      if (SetController(port, portAddress, controllerId, bProvidesInput))
        return true;
    }
  }

  return false;
}

bool CControllerTopology::SetController(const PortPtr &port, const std::string &portAddress, const std::string &controllerId, bool bProvidesInput)
{
  bool bSuccess = false;

  std::string portId;
  std::string remainingAddress;
  SplitAddress(portAddress, portId, remainingAddress);

  if (port->portId == portId)
  {
    const auto &accepts = port->accepts;

    if (remainingAddress.empty())
    {
      // Base case
      auto it = std::find_if(accepts.begin(), accepts.end(),
        [&controllerId](const ControllerPtr &controller)
        {
          return controllerId == controller->controllerId;
        });

      if (it != accepts.end())
      {
        port->activeId = controllerId;
        (*it)->bProvidesInput = bProvidesInput;
        bSuccess = true;
      }
    }
    else
    {
      const ControllerPtr& controller = GetActiveController(port);
      if (controller)
      {
        if (SetController(controller, remainingAddress, controllerId, bProvidesInput))
          bSuccess = true;
      }
    }
  }

  return bSuccess;
}

bool CControllerTopology::SetController(const ControllerPtr &controller, const std::string &portAddress, const std::string &controllerId, bool bProvidesInput)
{
  std::string portControllerId;
  std::string remainingAddress;
  SplitAddress(portAddress, portControllerId, remainingAddress);

  if (controller->controllerId == portControllerId)
  {
    const auto &ports = controller->ports;
    for (const auto &port : ports)
    {
      if (SetController(port, remainingAddress, controllerId, bProvidesInput))
        return true;
    }
  }

  return false;
}

void CControllerTopology::RemoveController(const std::string &portAddress)
{
  for (const auto &port : m_ports)
  {
    if (port->type == GAME_PORT_CONTROLLER)
      RemoveController(port, portAddress);
  }
}

void CControllerTopology::RemoveController(const PortPtr &port, const std::string &portAddress)
{
  std::string portId;
  std::string remainingAddress;
  SplitAddress(portAddress, portId, remainingAddress);

  if (port->portId == portId)
  {
    if (remainingAddress.empty())
    {
      // Base case
      port->activeId.clear();
    }
    else
    {
      const ControllerPtr& controller = GetActiveController(port);
      if (controller)
        RemoveController(controller, remainingAddress);
    }
  }
}

void CControllerTopology::RemoveController(const ControllerPtr &controller, const std::string &portAddress)
{
  std::string portControllerId;
  std::string remainingAddress;
  SplitAddress(portAddress, portControllerId, remainingAddress);

  if (controller->controllerId == portControllerId)
  {
    const auto &ports = controller->ports;
    for (const auto &port : ports)
      RemoveController(port, remainingAddress);
  }
}

libretro_device_t CControllerTopology::TypeOverride(const std::string& portAddress, const std::string& controllerId) const
{
  return TypeOverride(m_ports, JoinAddress(portAddress, controllerId));
}

libretro_device_t CControllerTopology::TypeOverride(const std::vector<PortPtr>& ports, const std::string &controllerAddress)
{
  for (const auto &port : ports)
  {
    if (port->type == GAME_PORT_CONTROLLER)
    {
      std::string portId;
      std::string remainingAddress;
      SplitAddress(controllerAddress, portId, remainingAddress);

      if (port->portId == portId)
        return TypeOverride(port->accepts, remainingAddress);
    }
  }

  return RETRO_DEVICE_NONE;
}

libretro_device_t CControllerTopology::TypeOverride(const std::vector<ControllerPtr>& controllers, const std::string &controllerAddress)
{
  std::string controllerId;
  std::string remainingAddress;
  SplitAddress(controllerAddress, controllerId, remainingAddress);

  if (remainingAddress.empty())
  {
    // Base case
    auto it = std::find_if(controllers.begin(), controllers.end(),
      [&controllerId](const ControllerPtr &controller)
      {
        return controllerId == controller->controllerId;
      });

    if (it != controllers.end())
      return (*it)->type;
  }
  else
  {
    for (const auto& controller : controllers)
    {
      if (controller->controllerId == controllerId)
        return TypeOverride(controller->ports, remainingAddress);
    }
  }

  return RETRO_DEVICE_NONE;
}

libretro_subclass_t CControllerTopology::SubclassOverride(const std::string& portAddress, const std::string& controllerId) const
{
  return SubclassOverride(m_ports, JoinAddress(portAddress, controllerId));
}

libretro_subclass_t CControllerTopology::SubclassOverride(const std::vector<PortPtr>& ports, const std::string &controllerAddress)
{
  for (const auto &port : ports)
  {
    if (port->type == GAME_PORT_CONTROLLER)
    {
      std::string portId;
      std::string remainingAddress;
      SplitAddress(controllerAddress, portId, remainingAddress);

      if (port->portId == portId)
        return SubclassOverride(port->accepts, remainingAddress);
    }
  }

  return RETRO_SUBCLASS_NONE;
}

libretro_subclass_t CControllerTopology::SubclassOverride(const std::vector<ControllerPtr>& controllers, const std::string &controllerAddress)
{
  std::string controllerId;
  std::string remainingAddress;
  SplitAddress(controllerAddress, controllerId, remainingAddress);

  if (remainingAddress.empty())
  {
    // Base case
    auto it = std::find_if(controllers.begin(), controllers.end(),
      [&controllerId](const ControllerPtr &controller)
      {
        return controllerId == controller->controllerId;
      });

    if (it != controllers.end())
      return (*it)->subclass;
  }
  else
  {
    for (const auto& controller : controllers)
    {
      if (controller->controllerId == controllerId)
        return SubclassOverride(controller->ports, remainingAddress);
    }
  }

  return RETRO_SUBCLASS_NONE;
}

bool CControllerTopology::Deserialize(const TiXmlElement* pElement)
{
  bool bSuccess = false;

  if (pElement == nullptr ||
      pElement->ValueStr() != TOPOLOGY_XML_ROOT)
  {
    esyslog("Can't find root <%s> tag", TOPOLOGY_XML_ROOT);
  }
  else
  {
    const char* strPlayerLimit = pElement->Attribute(TOPOLOGY_XML_ATTR_PLAYER_LIMIT);
    if (strPlayerLimit != nullptr)
    {
      std::istringstream ssPlayerLimit(strPlayerLimit);
      ssPlayerLimit >> m_playerLimit;
    }

    const TiXmlElement* pChild = pElement->FirstChildElement(TOPOLOGY_XML_ELEM_PORT);
    if (pChild == nullptr)
    {
      esyslog("Can't find <%s> tag", TOPOLOGY_XML_ELEM_PORT);
    }
    else
    {
      bSuccess = true;

      for ( ; pChild != nullptr; pChild = pChild->NextSiblingElement(TOPOLOGY_XML_ELEM_PORT))
      {
        PortPtr port = DeserializePort(pChild);

        if (!port)
        {
          bSuccess = false;
          break;
        }

        m_ports.emplace_back(std::move(port));
      }

      if (bSuccess)
        dsyslog("Loaded controller topology with %u ports", m_ports.size());
    }
  }

  return bSuccess;
}

CControllerTopology::PortPtr CControllerTopology::DeserializePort(const TiXmlElement* pElement)
{
  PortPtr port;

  const char* strPortType = pElement->Attribute(TOPOLOGY_XML_ATTR_PORT_TYPE);
  GAME_PORT_TYPE portType = CInputTranslator::GetPortType(strPortType != nullptr ? strPortType : "");

  //! @todo Remove this hack
  if (portType == GAME_PORT_UNKNOWN)
    portType = GAME_PORT_CONTROLLER;

  std::string portId;
  switch (portType)
  {
  case GAME_PORT_CONTROLLER:
  {
    const char* strPortId = pElement->Attribute(TOPOLOGY_XML_ATTR_PORT_ID);
    if (strPortId != nullptr)
      portId = strPortId;
    else
      esyslog("<%s> tag is missing attribute \"%s\", can't proceed without port ID", TOPOLOGY_XML_ELEM_PORT, TOPOLOGY_XML_ATTR_PORT_ID);
    break;
  }
  case GAME_PORT_KEYBOARD:
  {
    portId = KEYBOARD_PORT_ID;
    break;
  }
  case GAME_PORT_MOUSE:
  {
    portId = MOUSE_PORT_ID;
    break;
  }
  default:
  {
    esyslog("<%s> tag attribute \"%s\" has unknown value: \"%s\"", TOPOLOGY_XML_ELEM_PORT, TOPOLOGY_XML_ATTR_PORT_TYPE, strPortType != nullptr ? strPortType : "");
    break;
  }
  }

  if (!portId.empty())
  {
    const char* strConnectionPort = pElement->Attribute(TOPOLOGY_XML_ATTR_CONNECTION_PORT);
    std::string connectionPort = strConnectionPort != nullptr ? strConnectionPort : "";

    const char* strForceConnected = pElement->Attribute(TOPOLOGY_XML_ATTR_FORCE_CONNECTED);
    bool forceConnected = (strForceConnected != nullptr && std::string(strForceConnected) == "true");

    port.reset(new Port{ portType, portId, std::move(connectionPort), forceConnected });

    const TiXmlElement* pChild = pElement->FirstChildElement(TOPOLOGY_XML_ELEM_ACCEPTS);
    if (pChild == nullptr)
    {
      dsyslog("<%s> tag with ID \"%s\" is missing <%s> node, port won't accept any controllers", TOPOLOGY_XML_ELEM_PORT, portId.c_str(), TOPOLOGY_XML_ELEM_ACCEPTS);
    }
    else
    {
      for ( ; pChild != nullptr; pChild = pChild->NextSiblingElement(TOPOLOGY_XML_ELEM_ACCEPTS))
      {
        ControllerPtr controller = DeserializeController(pChild);

        if (!controller)
        {
          port.reset();
          break;
        }

        port->accepts.emplace_back(std::move(controller));
      }
    }
  }

  return port;
}

CControllerTopology::ControllerPtr CControllerTopology::DeserializeController(const TiXmlElement* pElement)
{
  ControllerPtr controller;

  const char* strControllerId = pElement->Attribute(TOPOLOGY_XML_ATTR_CONTROLLER_ID);
  if (strControllerId == nullptr)
  {
    esyslog("<%s> tag is missing attribute \"%s\", can't proceed without controller ID", TOPOLOGY_XML_ELEM_ACCEPTS, TOPOLOGY_XML_ATTR_CONTROLLER_ID);
  }
  else
  {
    // Device type and subclass overrides
    libretro_device_t type = RETRO_DEVICE_NONE;
    libretro_subclass_t subclass = RETRO_SUBCLASS_NONE;

    const char* strType = pElement->Attribute(TOPOLOGY_XML_ATTR_DEVICE_TYPE);
    if (strType != nullptr)
      type = LibretroTranslator::GetDeviceType(strType);

    const char* strSubclass = pElement->Attribute(TOPOLOGY_XML_ATTR_DEVICE_SUBCLASS);
    if (strSubclass != nullptr)
      std::istringstream(strSubclass) >> subclass;

    controller.reset(new Controller{ strControllerId, {}, {}, type, subclass });

    const TiXmlElement* pChild = pElement->FirstChildElement(TOPOLOGY_XML_ELEM_PORT);
    for ( ; pChild != nullptr; pChild = pChild->NextSiblingElement(TOPOLOGY_XML_ELEM_PORT))
    {
      PortPtr port = DeserializePort(pChild);

      if (!port)
      {
        controller.reset();
        break;
      }

      controller->ports.emplace_back(std::move(port));
    }
  }

  return controller;
}

game_input_port *CControllerTopology::GetPorts(const std::vector<PortPtr> &portVec, unsigned int &portCount)
{
  game_input_port *ports = nullptr;

  portCount = static_cast<unsigned int>(portVec.size());
  if (portCount > 0)
  {
    ports = new game_input_port[portCount];

    for (unsigned int i = 0; i < portCount; i++)
    {
      ports[i].type = portVec[i]->type;
      ports[i].port_id = portVec[i]->portId.c_str();
      ports[i].force_connected = portVec[i]->forceConnected;

      unsigned int deviceCount = 0;
      ports[i].accepted_devices = GetControllers(portVec[i]->accepts, deviceCount);
      ports[i].device_count = deviceCount;
    }
  }

  return ports;
}

game_input_device *CControllerTopology::GetControllers(const std::vector<ControllerPtr> &controllerVec, unsigned int &deviceCount)
{
  game_input_device *devices = nullptr;

  deviceCount = static_cast<unsigned int>(controllerVec.size());
  if (deviceCount > 0)
  {
    devices = new game_input_device[deviceCount];

    for (unsigned int i = 0; i < deviceCount; i++)
    {
      devices[i].controller_id = controllerVec[i]->controllerId.c_str();

      unsigned int portCount = 0;
      devices[i].available_ports = GetPorts(controllerVec[i]->ports, portCount);
      devices[i].port_count = portCount;
    }
  }

  return devices;
}

void CControllerTopology::FreeControllers(game_input_device *devices, unsigned int deviceCount)
{
  for (unsigned int i = 0; i < deviceCount; i++)
    FreePorts(devices[i].available_ports, devices[i].port_count);

  delete[] devices;
}

CControllerTopology::PortPtr CControllerTopology::CreateDefaultPort(const std::string &acceptedController)
{
  PortPtr port(new Port{GAME_PORT_CONTROLLER, DEFAULT_PORT_ID, {}, {}});

  port->accepts.emplace_back(new Controller{ acceptedController });

  return port;
}

const CControllerTopology::ControllerPtr& CControllerTopology::GetActiveController(const PortPtr& port)
{
  if (!port->activeId.empty())
  {
    const auto &accepts = port->accepts;

    auto it = std::find_if(accepts.begin(), accepts.end(),
      [&port](const ControllerPtr &controller)
      {
        return port->activeId == controller->controllerId;
      });

    if (it != accepts.end())
      return *it;
  }

  static const ControllerPtr empty;
  return empty;
}

void CControllerTopology::SplitAddress(const std::string &address, std::string &nodeId, std::string &remainingAddress)
{
  // Start searching after leading /
  size_t pos = address.find(ADDRESS_SEPARATOR, 1);
  if (pos == std::string::npos)
  {
    // Skip leading / to extract node ID
    nodeId = address.substr(1);
    remainingAddress.clear();
  }
  else
  {
    // Skip leading / to extract node ID
    nodeId = address.substr(1, pos - 1);
    remainingAddress = address.substr(pos);
  }
}

std::string CControllerTopology::JoinAddress(const std::string& address, const std::string& nodeId)
{
  return address + ADDRESS_SEPARATOR + nodeId;
}
