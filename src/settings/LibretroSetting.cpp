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

#include "LibretroSetting.h"
#include "libretro/libretro.h"

using namespace LIBRETRO;

CLibretroSetting::CLibretroSetting(const retro_variable* libretroVariable) :
  m_key(libretroVariable->key)
{
  Parse(libretroVariable->value);

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
        pos++;
        while (pos < retroVal.size() && retroVal[pos] == ' ')
          pos++;
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
        if ((pos = strValues.find('|')) == std::string::npos)
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
