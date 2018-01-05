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

#include "kodi_game_types.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class TiXmlElement;

namespace LIBRETRO
{
  class CLibretroDeviceInput;

  class CLibretroDevice
  {
  public:
    CLibretroDevice(const game_controller* controller);
    ~CLibretroDevice();

    const std::string &ControllerID(void) const { return m_controllerId; }
    bool HasModel(const std::string &model) const;
    const ModelInfo &DefaultModel() const { return m_defaultModel; }
    const ModelMap &Models() const { return m_modelMap; }
    const ModelInfo &GetModel(const std::string &model) const;
    libretro_device_t Type(void) const { return m_type; }
    const FeatureMap& Features(void) const { return m_featureMap; }
    CLibretroDeviceInput& Input() { return *m_input; }

    bool Deserialize(const TiXmlElement* pElement, unsigned int buttonMapVersion);

  private:
    static bool DeserializeModel(const TiXmlElement* pElement, ModelInfo &modelInfo);

    std::string                            m_controllerId;
    libretro_device_t                      m_type;
    FeatureMap                             m_featureMap;
    std::unique_ptr<CLibretroDeviceInput>  m_input;
    ModelInfo                              m_defaultModel;
    ModelMap                               m_modelMap;
  };
}
