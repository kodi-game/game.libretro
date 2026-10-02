/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
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

    /*!
     * \brief Build a setting from the core options API
     *
     * The newer API hands the pieces over separately rather than in one
     * semicolon-and-pipe string, and names its own default rather than relying
     * on the first value being it.
     */
    CLibretroSetting(const char* key,
                     const char* description,
                     std::vector<std::string> values,
                     const char* defaultValue,
                     const char* info = nullptr,
                     const char* category = nullptr,
                     std::vector<std::string> labels = {});

    const std::string&              Key() const          { return m_key; }
    const std::string&              Description() const  { return m_description; }
    const std::string&              Info() const         { return m_info; }
    const std::string&              Category() const     { return m_category; }
    const std::vector<std::string>& Values() const       { return m_values; }
    const std::vector<std::string>& Labels() const       { return m_labels; } // Display text by value, where the core gave any
    const std::string&              ValuesStr() const    { return m_valuesStr; } // Original pipe-deliminated values string
    const std::string&              CurrentValue() const { return m_currentValue; }

    // The value the core names as its default, or else its first value, which
    // is the default a retro_variable implies
    const std::string& DefaultValue() const;

    void SetCurrentValue(const std::string& newValue) { m_currentValue = newValue; }

  private:
    void Parse(const std::string& libretroValue);

    std::string              m_key;
    std::string              m_description;
    std::string              m_info;
    std::string              m_category;
    std::vector<std::string> m_values;
    std::vector<std::string> m_labels;
    std::string              m_valuesStr;
    std::string              m_defaultValue;
    std::string              m_currentValue;
  };
} // namespace LIBRETRO
