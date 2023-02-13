/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "InputTypes.h"

#include <kodi/addon-instance/Game.h>

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
    CLibretroDevice();
    CLibretroDevice(const std::string &controller);
    ~CLibretroDevice();

    const std::string& ControllerID(void) const { return m_controllerId; }
    libretro_device_t Type(void) const { return m_type; }
    libretro_subclass_t Subclass() const { return m_subclass; }
    const FeatureMap& Features(void) const { return m_featureMap; }
    CLibretroDeviceInput& Input() { return *m_input; }

    void SetType(libretro_device_t type) { m_type = type; }
    void SetSubclass(libretro_subclass_t subclass) { m_subclass = subclass; }

    bool Deserialize(const TiXmlElement* pElement, unsigned int buttonMapVersion);

  private:
    std::string                            m_controllerId;
    libretro_device_t                      m_type;
    libretro_subclass_t                    m_subclass = RETRO_SUBCLASS_NONE;
    FeatureMap                             m_featureMap;
    std::unique_ptr<CLibretroDeviceInput>  m_input;
  };
}
