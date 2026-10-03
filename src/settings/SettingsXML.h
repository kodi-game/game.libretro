/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <map>
#include <string>

namespace LIBRETRO
{
  /*!
   * \brief game.libretro's own settings, read from the file Kodi keeps them in
   *
   * Kodi only gives a core its own add-on's settings, never these.
   */
  class CSettingsXML
  {
  public:
    /*!
     * \brief Read the settings file
     *
     * Until the settings are first saved there is no file, and every setting
     * keeps its default.
     */
    void Load();

    bool GetBool(const std::string& id, bool defaultValue) const;
    std::string GetString(const std::string& id, const std::string& defaultValue) const;

  private:
    std::map<std::string, std::string> m_values;
  };
}
