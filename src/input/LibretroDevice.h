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

#include "kodi/kodi_game_types.h"

#include <memory>
#include <string>
#include <vector>

namespace LIBRETRO
{
  typedef unsigned int libretro_device_t;

  class CLibretroDevice;
  typedef std::shared_ptr<CLibretroDevice> DevicePtr;

  class CLibretroDeviceInput;

  class CLibretroDevice
  {
  public:
    CLibretroDevice(const game_controller* controller);

    std::string ControllerID(void) const { return m_controllerId; }
    libretro_device_t Type(void) const { return m_type; }
    CLibretroDeviceInput& Input() { return *m_input; }

  private:
    std::string                            m_controllerId;
    libretro_device_t                      m_type;
    //CLibretroDeviceMap                     m_libretroMap;
    std::unique_ptr<CLibretroDeviceInput>  m_input;
  };
}
