/*
 *      Copyright (C) 2018 Team Kodi
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

#include <string>
#include <vector>

struct game_controller_layout;

namespace LIBRETRO
{
  class CControllerLayout
  {
  public:
    CControllerLayout(const game_controller_layout &controller);

    const std::string &ControllerID() const { return m_controllerId; }
    bool ProvidesInput() const { return m_bProvidesInput; }

  private:
    std::string m_controllerId;
    bool m_bProvidesInput;
    std::vector<std::string> m_digitalButtons;
    std::vector<std::string> m_analogButtons;
    std::vector<std::string> m_analogSticks;
    std::vector<std::string> m_accelerometers;
    std::vector<std::string> m_keys;
    std::vector<std::string> m_relPointers;
    std::vector<std::string> m_absPointers;
    std::vector<std::string> m_motors;
  };
}
