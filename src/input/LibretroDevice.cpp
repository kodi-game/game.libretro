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

#include "LibretroDevice.h"
#include "ButtonMapper.h"
#include "InputDefinitions.h"
#include "LibretroDeviceInput.h"
#include "libretro/LibretroTranslator.h"
#include "libretro/libretro.h"
#include "log/Log.h"

#include "tinyxml.h"

using namespace LIBRETRO;

CLibretroDevice::CLibretroDevice(const game_controller* controller)
  : m_type(RETRO_DEVICE_NONE),
    m_input(new CLibretroDeviceInput(controller))
{
  if (controller && controller->controller_id)
  {
    m_controllerId = controller->controller_id;
    m_type = CButtonMapper::Get().GetLibretroType(m_controllerId);
  }
}

CLibretroDevice::~CLibretroDevice()
{
}

bool CLibretroDevice::Deserialize(const TiXmlElement* pElement)
{
  if (!pElement)
    return false;

  const char* controllerId = pElement->Attribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID);
  if (!controllerId)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELM_CONTROLLER, BUTTONMAP_XML_ATTR_CONTROLLER_ID);
    return false;
  }

  const char* type = pElement->Attribute(BUTTONMAP_XML_ATTR_CONTROLLER_TYPE);
  if (!type)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELM_CONTROLLER, BUTTONMAP_XML_ATTR_CONTROLLER_TYPE);
    return false;
  }

  m_controllerId = controllerId;
  m_type = LibretroTranslator::GetDeviceType(type);

  const TiXmlElement* pFeature = pElement->FirstChildElement(BUTTONMAP_XML_ELM_FEATURE);
  if (!pFeature)
  {
    esyslog("Can't find <%s> tag for controller \"%s\"", BUTTONMAP_XML_ELM_FEATURE, m_controllerId.c_str());
    return false;
  }

  while (pFeature)
  {
    const char* name = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_NAME);
    if (!name)
    {
      esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_NAME);
      return false;
    }

    const char* mapto = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_MAPTO);
    if (!mapto)
    {
      esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_MAPTO);
      return false;
    }

    m_featureMap[name] = mapto;

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELM_FEATURE);
  }

  return true;
}
