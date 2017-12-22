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

#include <map>
#include <memory>
#include <string>

// No subclass
#define RETRO_SUBCLASS_NONE  (-1)

namespace LIBRETRO
{
  class CLibretroDevice;
  using DevicePtr = std::shared_ptr<CLibretroDevice>;

  using libretro_device_t = unsigned int;
  using libretro_subclass_t = int;

  struct FeatureMapItem
  {
    std::string feature;
    std::string axis;
  };
  using FeatureMap = std::map<std::string, FeatureMapItem>;

  struct ModelInfo
  {
    libretro_subclass_t subclass = RETRO_SUBCLASS_NONE;
  };
  typedef std::map<std::string, ModelInfo> ModelMap;
}
