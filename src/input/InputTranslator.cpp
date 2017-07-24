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
