/*
 *  Copyright (C) 2015-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroDevice.h"
#include "ButtonMapper.h"
#include "InputDefinitions.h"
#include "LibretroDeviceInput.h"
#include "libretro/LibretroTranslator.h"
#include "libretro/libretro.h"
#include "log/Log.h"

#include <tinyxml.h>

#include <sstream>

using namespace LIBRETRO;

CLibretroDevice::CLibretroDevice() :
  m_type(RETRO_DEVICE_NONE)
{
}

CLibretroDevice::CLibretroDevice(const std::string &controllerId)
  : m_controllerId(controllerId),
    m_type(CButtonMapper::Get().GetLibretroType(controllerId)),
    m_subclass(CButtonMapper::Get().GetSubclass(controllerId)),
    m_input(new CLibretroDeviceInput(controllerId))
{
}

CLibretroDevice::~CLibretroDevice()
{
}

bool CLibretroDevice::Deserialize(const TiXmlElement* pElement, unsigned int buttonMapVersion)
{
  if (!pElement)
    return false;

  // Controller ID
  const char* controllerId = pElement->Attribute(BUTTONMAP_XML_ATTR_CONTROLLER_ID);
  if (!controllerId)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELM_CONTROLLER, BUTTONMAP_XML_ATTR_CONTROLLER_ID);
    return false;
  }

  // Device type
  const char* type = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_TYPE);
  if (!type)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELM_CONTROLLER, BUTTONMAP_XML_ATTR_DEVICE_TYPE);
    return false;
  }

  m_controllerId = controllerId;
  m_type = LibretroTranslator::GetDeviceType(type);

  if (m_type == RETRO_DEVICE_NONE)
  {
    esyslog("<%s> tag has invalid device type: \"%s\"", BUTTONMAP_XML_ELM_CONTROLLER, type);
    return false;
  }

  // Device subclass
  const char* subclass = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_SUBCLASS);
  if (subclass)
    std::istringstream(subclass) >> m_subclass;
  else
    m_subclass = RETRO_SUBCLASS_NONE;

  // Features
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

    const char* axis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS);

    FeatureMapItem libretroFeature = { mapto };

    // Ensure feature is valid
    if (LibretroTranslator::GetFeatureIndex(libretroFeature.feature) < 0)
    {
      esyslog("<%s> tag has invalid \"%s\" attribute: \"%s\"", BUTTONMAP_XML_ELM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_MAPTO, mapto);
      return false;
    }

    if (axis != nullptr)
    {
      libretroFeature.axis = axis;

      // Ensure axis is valid
      if (LibretroTranslator::GetAxisID(libretroFeature.axis) < 0)
      {
        esyslog("<%s> tag has invalid \"%s\" attribute: \"%s\"", BUTTONMAP_XML_ELM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_AXIS, axis);
        return false;
      }
    }

    m_featureMap[name] = std::move(libretroFeature);

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELM_FEATURE);
  }

  return true;
}
