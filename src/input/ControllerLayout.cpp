/*
 *  Copyright (C) 2018-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "ControllerLayout.h"

using namespace LIBRETRO;

CControllerLayout::CControllerLayout(const kodi::addon::GameControllerLayout& controller) :
  m_controller(controller)
{
}
