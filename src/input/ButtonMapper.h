/*
 *  Copyright (C) 2015-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "InputTypes.h"

#include <vector>
#include <string>

// TODO: Make this class generic and move XML-specific stuff to xml subfolder
class TiXmlElement;

namespace LIBRETRO
{
  class CButtonMapper
  {
  private:
    CButtonMapper(void);

  public:
    static CButtonMapper& Get(void);

    bool LoadButtonMap(void);

    libretro_device_t GetLibretroType(const std::string& strControllerId);

    libretro_subclass_t GetSubclass(const std::string& strControllerId);

    int GetLibretroIndex(const std::string& strControllerId, const std::string& strFeatureName);
    libretro_device_t GetLibretroDevice(const std::string& strControllerId, const std::string& strFeatureName) const;
    int GetAxisID(const std::string& strControllerId, const std::string& strFeatureName) const;

    std::string GetControllerFeature(const std::string& strControllerId, const std::string& strLibretroFeature);

  private:
    bool HasController(const std::string& strControllerId) const;
    std::string GetFeature(const std::string& strControllerId, const std::string& strFeatureName) const;
    std::string GetAxis(const std::string& strControllerId, const std::string& strFeatureName) const;

    bool Deserialize(TiXmlElement* pElement);

    using DeviceVector = std::vector<DevicePtr>;
    using DeviceIt = DeviceVector::const_iterator;

    static DeviceIt GetDevice(const DeviceVector &devices,
                              const std::string &controllerId);

    bool                   m_bLoadAttempted;
    DeviceVector m_devices;
  };
}
