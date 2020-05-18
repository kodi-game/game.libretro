/*
 *  Copyright (C) 2017-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>

#define DEFAULT_CONTROLLER_ID  "game.controller.default"

namespace LIBRETRO
{
  class CDefaultControllerTranslator
  {
  public:
    /*!
     * \brief Translate from Kodi feature name to libretro index
     */
    static int GetLibretroIndex(const std::string &strFeatureName);

    /*!
     * \brief Translate from libretro feature (from libretro.h) to Kodi feature
     *
     * This is necessary because input doesn't just flow from Kodi to the
     * add-on. Rumble feedback going from the add-on to Kodi makes use of this
     * functionality.
     */
    static std::string GetControllerFeature(const std::string &strLibretroFeature);
  };
}
