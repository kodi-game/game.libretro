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

#include "ButtonMapper.h"
#include "DefaultControllerTranslator.h"
#include "InputDefinitions.h"
#include "LibretroDevice.h"
#include "libretro/LibretroDLL.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/LibretroTranslator.h"
#include "libretro/libretro.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <sstream>

#define BUTTONMAP_XML          "buttonmap.xml"
#define DEFAULT_CONTROLLER_ID  "game.controller.default"

using namespace LIBRETRO;

CButtonMapper::CButtonMapper(void)
  : m_bLoadAttempted(false)
{
}

CButtonMapper& CButtonMapper::Get(void)
{
  static CButtonMapper _instance;
  return _instance;
}

bool CButtonMapper::LoadButtonMap(void)
{
  bool bSuccess = false;

  m_devices.clear();

  std::string strFilename = CLibretroEnvironment::Get().GetResourcePath(BUTTONMAP_XML);
  if (strFilename.empty())
  {
    esyslog("Could not locate buttonmap \"%s\"", BUTTONMAP_XML);
  }
  else
  {
    dsyslog("Loading libretro buttonmap %s", strFilename.c_str());

    TiXmlDocument buttonMapXml;
    if (!buttonMapXml.LoadFile(strFilename))
    {
      esyslog("Failed to open file: %s", strFilename.c_str());
    }
    else
    {
      TiXmlElement* pRootElement = buttonMapXml.RootElement();
      bSuccess = Deserialize(pRootElement);
    }
  }

  return bSuccess;
}

libretro_device_t CButtonMapper::GetLibretroType(const std::string& strControllerId)
{
  // Handle default controller
  if (strControllerId == DEFAULT_CONTROLLER_ID)
    return RETRO_DEVICE_ANALOG;

  libretro_device_t deviceType = RETRO_DEVICE_NONE;

  for (auto& device : m_devices)
  {
    if (device->ControllerID() == strControllerId)
    {
      deviceType = device->Type();
      break;
    }
  }

  return deviceType;
}

int CButtonMapper::GetLibretroIndex(const std::string& strControllerId, const std::string& strFeatureName)
{
  if (!strControllerId.empty() && !strFeatureName.empty())
  {
    // Handle default controller unless it appears in buttonmap.xml
    if (strControllerId == DEFAULT_CONTROLLER_ID && !HasController(DEFAULT_CONTROLLER_ID))
      return CDefaultControllerTranslator::GetLibretroIndex(strFeatureName);

    // Check buttonmap for other controllers
    std::string mapto = GetFeature(strControllerId, strFeatureName);
    if (!mapto.empty())
      return LibretroTranslator::GetFeatureIndexV2(mapto);
  }

  return -1;
}

std::string CButtonMapper::GetControllerFeature(const std::string& strControllerId, const std::string& strLibretroFeature)
{
  std::string feature;

  if (!strControllerId.empty() && !strLibretroFeature.empty())
  {
    // Handle default controller unless it appears in buttonmap.xml
    if (strControllerId == DEFAULT_CONTROLLER_ID && !HasController(DEFAULT_CONTROLLER_ID))
      return CDefaultControllerTranslator::GetControllerFeature(strLibretroFeature);

    for (auto& device : m_devices)
    {
      if (device->ControllerID() == strControllerId)
      {
        const FeatureMap& features = device->Features();
        for (auto& featurePair : features)
        {
          const std::string& controllerFeature = featurePair.first;
          const std::string& libretroFeature = featurePair.second;

          if (libretroFeature == strLibretroFeature)
          {
            feature = controllerFeature;
            break;
          }
        }
        break;
      }
    }
  }

  return feature;
}

bool CButtonMapper::HasController(const std::string& strControllerId) const
{
  bool bFound = false;

  for (auto& device : m_devices)
  {
    if (device->ControllerID() == strControllerId)
    {
      bFound = true;
      break;
    }
  }

  return bFound;
}

std::string CButtonMapper::GetFeature(const std::string& strControllerId, const std::string& strFeatureName) const
{
  std::string mapto;

  for (auto& device : m_devices)
  {
    if (device->ControllerID() == strControllerId)
    {
      const FeatureMap& features = device->Features();
      for (auto& featurePair : features)
      {
        const std::string& controllerFeature = featurePair.first;
        const std::string& libretroFeature = featurePair.second;

        if (controllerFeature == strFeatureName)
        {
          mapto = libretroFeature;
          break;
        }
      }
      break;
    }
  }

  return mapto;
}

bool CButtonMapper::Deserialize(TiXmlElement* pElement)
{
  bool bSuccess = false;

  if (pElement == nullptr ||
      pElement->ValueStr() != BUTTONMAP_XML_ROOT)
  {
    esyslog("Can't find root <%s> tag", BUTTONMAP_XML_ROOT);
  }
  else
  {
    // Check buttonmap version (assume v1 if unspecified)
    unsigned int version = 1;

    const char* strVersion = pElement->Attribute(BUTTONMAP_XML_ATTR_VERSION);
    if (strVersion != nullptr)
    {
      std::istringstream ss(strVersion);
      ss >> version;
      dsyslog("Detected buttonmap version %u", version);
    }
    else
    {
      dsyslog("No version detected, defaulting to version 1");
    }

    const TiXmlElement* pChild = pElement->FirstChildElement(BUTTONMAP_XML_ELM_CONTROLLER);
    if (!pChild)
    {
      esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELM_CONTROLLER);
    }
    else
    {
      bSuccess = true;

      for ( ; pChild != nullptr; pChild = pChild->NextSiblingElement(BUTTONMAP_XML_ELM_CONTROLLER))
      {
        DevicePtr device = std::make_shared<CLibretroDevice>(nullptr);

        if (!device->Deserialize(pChild, version))
        {
          bSuccess = false;
          break;
        }

        m_devices.emplace_back(std::move(device));
      }

      if (bSuccess)
        dsyslog("Loaded buttonmap at version %u", version);
    }
  }

  return bSuccess;
}
