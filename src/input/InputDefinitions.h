/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

// Buttonmap XML
#define BUTTONMAP_XML_ROOT                  "buttonmap"
#define BUTTONMAP_XML_ELM_CONTROLLER        "controller"
#define BUTTONMAP_XML_ELM_FEATURE           "feature"
#define BUTTONMAP_XML_ATTR_VERSION          "version"
#define BUTTONMAP_XML_ATTR_CONTROLLER_ID    "id"
#define BUTTONMAP_XML_ATTR_DEVICE_TYPE      "type"
#define BUTTONMAP_XML_ATTR_DEVICE_SUBCLASS  "subclass"
#define BUTTONMAP_XML_ATTR_FEATURE_NAME     "name"
#define BUTTONMAP_XML_ATTR_FEATURE_MAPTO    "mapto"
#define BUTTONMAP_XML_ATTR_FEATURE_AXIS     "axis"

// Topology XML
#define TOPOLOGY_XML_ROOT                   "logicaltopology"
#define TOPOLOGY_XML_ELEM_PORT              "port"
#define TOPOLOGY_XML_ELEM_ACCEPTS           "accepts"
#define TOPOLOGY_XML_ATTR_PLAYER_LIMIT      "playerlimit"
#define TOPOLOGY_XML_ATTR_PORT_TYPE         "type"
#define TOPOLOGY_XML_ATTR_PORT_ID           "id"
#define TOPOLOGY_XML_ATTR_CONNECTION_PORT   "connectionPort"
#define TOPOLOGY_XML_ATTR_CONTROLLER_ID     "controller"

// Game API strings
#define PORT_TYPE_KEYBOARD    "keyboard"
#define PORT_TYPE_MOUSE       "mouse"
#define PORT_TYPE_CONTROLLER  "controller"
