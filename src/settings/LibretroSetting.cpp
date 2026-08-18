/*
 *  Copyright (C) 2016-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LibretroSetting.h"
#include "libretro-common/libretro.h"

#include <algorithm>

using namespace LIBRETRO;

CLibretroSetting::CLibretroSetting(const retro_variable* libretroVariable) :
  m_key(libretroVariable->key)
{
  Parse(libretroVariable->value);

  SetCurrentValue(DefaultValue());
}

CLibretroSetting::CLibretroSetting(const char* key,
                                   const char* description,
                                   std::vector<std::string> values,
                                   const char* defaultValue)
  : m_key(key != nullptr ? key : ""),
    m_description(description != nullptr ? description : ""),
    m_values(std::move(values))
{
  // Kept for the add-on's generated settings, which are written in the older
  // pipe-delimited form whichever API the core used to declare them
  for (const std::string& value : m_values)
  {
    if (!m_valuesStr.empty())
      m_valuesStr += "|";
    m_valuesStr += value;
  }

  // Unlike a retro_variable, this API names its default rather than leaving it
  // implied by ordering. Fall back to the first value if the named one is not
  // among them.
  if (defaultValue != nullptr &&
      std::find(m_values.begin(), m_values.end(), defaultValue) != m_values.end())
    SetCurrentValue(defaultValue);
  else
    SetCurrentValue(DefaultValue());
}

const std::string& CLibretroSetting::DefaultValue() const
{
  static std::string empty;

  if (!m_values.empty())
    return m_values[0];

  return empty;
}

void CLibretroSetting::Parse(const std::string& libretroValue)
{
  // Example retro_variable:
  //      { "foo_option", "Speed hack coprocessor X; false|true" }
  //
  // Text before first ';' is description. This ';' must be followed by
  // a space, and followed by a list of possible values split up with '|'.

  // First, Split the string into description and value list
  auto SplitDescription = [](const std::string& retroVal, std::string& description, std::string& values)
    {
      // Look for ; separating the description from the pipe-separated values
      size_t pos;
      if ((pos = retroVal.find(';')) != std::string::npos)
      {
        // Set description
        description = retroVal.substr(0, pos);

        // Advance past semicolon
        pos++;

        // Advance past spaces
        while (pos < retroVal.size() && retroVal[pos] == ' ')
          pos++;

        // Set values
        values = retroVal.substr(pos);
      }
      else
      {
        // No semicolon found
        values = retroVal;
      }
    };

  std::string strDescription;
  std::string strValues;
  SplitDescription(libretroValue, strDescription, strValues);

  if (strDescription.empty())
  {
    // Use key name as a backup
    strDescription = m_key;
  }

  // Split the values on | delimiter
  auto SplitValues = [](const std::string& strValues, std::vector<std::string>& vecValues)
    {
      std::string remainingValues = strValues;
      while (!remainingValues.empty())
      {
        std::string strValue;

        size_t pos;
        if ((pos = remainingValues.find('|')) == std::string::npos)
        {
          strValue = remainingValues;
          remainingValues.clear();
        }
        else
        {
          strValue = remainingValues.substr(0, pos);
          remainingValues.erase(0, pos + 1);
        }

        vecValues.push_back(strValue);
      }
    };

  std::vector<std::string> vecValues;
  SplitValues(strValues, vecValues);

  m_description = std::move(strDescription);
  m_values = std::move(vecValues);
  m_valuesStr = std::move(strValues);
}
