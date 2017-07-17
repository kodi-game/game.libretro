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

#include <string>

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
