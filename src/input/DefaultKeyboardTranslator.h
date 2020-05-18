/*
 *  Copyright (C) 2018-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>

#define DEFAULT_KEYBOARD_ID    "game.controller.keyboard"

namespace LIBRETRO
{
  class CDefaultKeyboardTranslator
  {
  public:
    /*!
     * \brief Translate from Kodi feature name to libretro index
     */
    static int GetLibretroIndex(const std::string &strFeatureName);
  };
}
