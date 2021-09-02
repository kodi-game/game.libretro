/*
 *  Copyright (C) 2017-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "InputTranslator.h"
#include "InputDefinitions.h"

using namespace LIBRETRO;

GAME_PORT_TYPE CInputTranslator::GetPortType(const std::string &portType)
{
  if (portType == PORT_TYPE_KEYBOARD)    return GAME_PORT_KEYBOARD;
  if (portType == PORT_TYPE_MOUSE)       return GAME_PORT_MOUSE;
  if (portType == PORT_TYPE_CONTROLLER)  return GAME_PORT_CONTROLLER;

  return GAME_PORT_UNKNOWN;
}
