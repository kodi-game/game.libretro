/*
 *  Copyright (C) 2018-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/addon-instance/Game.h>

#include <string>
#include <vector>

struct game_controller_layout;

namespace LIBRETRO
{
  class CControllerLayout
  {
  public:
    CControllerLayout(const kodi::addon::GameControllerLayout& controller);

    const std::string &ControllerID() const { return m_controller.controller_id; }
    bool ProvidesInput() const { return m_controller.provides_input; }

  private:
    kodi::addon::GameControllerLayout m_controller;
  };
}
