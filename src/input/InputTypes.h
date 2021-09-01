/*
 *  Copyright (C) 2017-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

// No subclass
#define RETRO_SUBCLASS_NONE  (-1)

namespace LIBRETRO
{
  class CLibretroDevice;
  using DevicePtr = std::shared_ptr<CLibretroDevice>;
  using DeviceVector = std::vector<DevicePtr>;

  using libretro_device_t = unsigned int;
  using libretro_subclass_t = int;

  struct FeatureMapItem
  {
    std::string feature;
    std::string axis;
  };
  using FeatureMap = std::map<std::string, FeatureMapItem>;
}
