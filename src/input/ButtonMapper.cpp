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
#include "InputDefinitions.h"
#include "LibretroDevice.h"
#include "libretro/LibretroDLL.h"
#include "libretro/LibretroEnvironment.h"
#include "libretro/LibretroTranslator.h"
#include "libretro/libretro.h"
#include "log/Log.h"

#include "tinyxml.h"

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
      if (pRootElement == nullptr ||
          pRootElement->NoChildren() ||
          pRootElement->ValueStr() != BUTTONMAP_XML_ROOT)
      {
        esyslog("Can't find root <%s> tag in %s", BUTTONMAP_XML_ROOT, strFilename.c_str());
      }
      else
      {
        const TiXmlElement* pChild = pRootElement->FirstChildElement(BUTTONMAP_XML_ELM_CONTROLLER);
        if (!pChild)
        {
          esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELM_CONTROLLER);
        }
        else
        {
          bSuccess = true;

          while (pChild)
          {
            DevicePtr device = std::make_shared<CLibretroDevice>(nullptr);
            if (!device->Deserialize(pChild))
            {
              bSuccess = false;
              break;
            }
            m_devices.emplace_back(std::move(device));
            pChild = pChild->NextSiblingElement(BUTTONMAP_XML_ELM_CONTROLLER);
          }
        }
      }
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
    {
      if (strFeatureName == "a")            return RETRO_DEVICE_ID_JOYPAD_A;
      if (strFeatureName == "b")            return RETRO_DEVICE_ID_JOYPAD_B;
      if (strFeatureName == "x")            return RETRO_DEVICE_ID_JOYPAD_X;
      if (strFeatureName == "y")            return RETRO_DEVICE_ID_JOYPAD_Y;
      if (strFeatureName == "start")        return RETRO_DEVICE_ID_JOYPAD_START;
      if (strFeatureName == "back")         return RETRO_DEVICE_ID_JOYPAD_SELECT;
      if (strFeatureName == "leftbumber")   return RETRO_DEVICE_ID_JOYPAD_L;
      if (strFeatureName == "rightbumper")  return RETRO_DEVICE_ID_JOYPAD_R;
      if (strFeatureName == "leftthumb")    return RETRO_DEVICE_ID_JOYPAD_L3;
      if (strFeatureName == "rightthumb")   return RETRO_DEVICE_ID_JOYPAD_R3;
      if (strFeatureName == "up")           return RETRO_DEVICE_ID_JOYPAD_UP;
      if (strFeatureName == "down")         return RETRO_DEVICE_ID_JOYPAD_DOWN;
      if (strFeatureName == "right")        return RETRO_DEVICE_ID_JOYPAD_RIGHT;
      if (strFeatureName == "left")         return RETRO_DEVICE_ID_JOYPAD_LEFT;
      if (strFeatureName == "lefttrigger")  return RETRO_DEVICE_ID_JOYPAD_L2;
      if (strFeatureName == "righttrigger") return RETRO_DEVICE_ID_JOYPAD_R2;
      if (strFeatureName == "leftstick")    return RETRO_DEVICE_INDEX_ANALOG_LEFT;
      if (strFeatureName == "rightstick")   return RETRO_DEVICE_INDEX_ANALOG_RIGHT;
      if (strFeatureName == "leftmotor")    return RETRO_RUMBLE_STRONG;
      if (strFeatureName == "rightmotor")   return RETRO_RUMBLE_WEAK;
    }

    // Check buttonmap for other controllers
    std::string mapto = GetFeature(strControllerId, strFeatureName);
    if (!mapto.empty())
      return LibretroTranslator::GetFeatureIndex(mapto);
  }

  return -1;
}

std::string CButtonMapper::GetControllerFeature(const std::string& strControllerId, const std::string& strLibretroFeature)
{
  std::string feature;

  if (!strControllerId.empty() && !strLibretroFeature.empty())
  {
    // Handle default controller unless it appears in buttonmap.xml
    if (strControllerId == DEFAULT_CONTROLLER_ID && HasController(DEFAULT_CONTROLLER_ID))
    {
      if (strLibretroFeature == "a")           return "a";
      if (strLibretroFeature == "b")           return "b";
      if (strLibretroFeature == "x")           return "x";
      if (strLibretroFeature == "y")           return "y";
      if (strLibretroFeature == "start")       return "start";
      if (strLibretroFeature == "select")      return "back";
      if (strLibretroFeature == "up")          return "up";
      if (strLibretroFeature == "down")        return "down";
      if (strLibretroFeature == "right")       return "right";
      if (strLibretroFeature == "left")        return "left";
      if (strLibretroFeature == "l")           return "leftbumber";
      if (strLibretroFeature == "r")           return "rightbumper";
      if (strLibretroFeature == "l2")          return "lefttrigger";
      if (strLibretroFeature == "r2")          return "righttrigger";
      if (strLibretroFeature == "l3")          return "leftthumb";
      if (strLibretroFeature == "r3")          return "rightthumb";
      if (strLibretroFeature == "leftstick")   return "leftstick";
      if (strLibretroFeature == "rightstick")  return "rightstick";
      if (strLibretroFeature == "strong")      return "leftmotor";
      if (strLibretroFeature == "weak")        return "rightmotor";
    }

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
