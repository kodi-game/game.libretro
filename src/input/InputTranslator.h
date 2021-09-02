/*
 *  Copyright (C) 2017-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/addon-instance/Game.h>

namespace LIBRETRO
{
  class CInputTranslator
  {
  public:
    /*!
     * \brief Translate from string to Game API enum
     */
    static GAME_PORT_TYPE GetPortType(const std::string &portType);
  };
}
