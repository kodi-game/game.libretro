/*
 *      Copyright (C) 2015-2016 Team Kodi
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

#include "InputTypes.h"

#include <memory>
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

    const ModelInfo &GetDefaultModel(const std::string& strControllerId) const;
    const ModelMap &GetModels(const std::string& strControllerId) const;

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
