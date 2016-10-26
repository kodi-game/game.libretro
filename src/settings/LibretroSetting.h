/*
 *      Copyright (C) 2016 Team Kodi
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

struct retro_variable;

namespace LIBRETRO
{
  class CLibretroSetting
  {
  public:
    CLibretroSetting(const retro_variable* libretroVariable);

    const std::string&              Key() const          { return m_key; }
    const std::string&              Description() const  { return m_description; }
    const std::vector<std::string>& Values() const       { return m_values; }
    const std::string&              ValuesStr() const    { return m_valuesStr; } // Original pipe-deliminated values string
    const std::string&              CurrentValue() const { return m_currentValue; }

    // The libretro API defaults the setting to its first value
    const std::string& DefaultValue() const;

    void SetCurrentValue(const std::string& newValue) { m_currentValue = newValue; }

  private:
    void Parse(const std::string& libretroValue);

    std::string              m_key;
    std::string              m_description;
    std::vector<std::string> m_values;
    std::string              m_valuesStr;
    std::string              m_currentValue;
  };
} // namespace LIBRETRO
