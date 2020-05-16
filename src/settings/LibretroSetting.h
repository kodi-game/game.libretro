/*
 *  Copyright (C) 2016-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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
