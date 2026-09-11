/*
 *  Copyright (C) 2018-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

struct retro_game_geometry;

namespace LIBRETRO
{
  class CVideoGeometry
  {
  public:
    CVideoGeometry() = default;
    CVideoGeometry(CVideoGeometry&) = default;
    CVideoGeometry(const retro_game_geometry &geometry);

    void UpdateVideoGeometry(const retro_game_geometry &geometry);

    unsigned int NominalWidth() const { return m_nominalWidth; }
    unsigned int NominalHeight() const { return m_nominalHeight; }
    unsigned int MaxWidth() const { return m_maxWidth; }
    unsigned int MaxHeight() const { return m_maxHeight; }
    float DisplayAspectRatio() const { return m_displayAspectRatio; }

    /*!
     * \brief Keep a maximum a core is not allowed to change
     *
     * SET_GEOMETRY carries max_width and max_height that libretro.h says are
     * ignored, so the ones already in force are put back over them.
     */
    void SetMaximum(unsigned int maxWidth, unsigned int maxHeight)
    {
      m_maxWidth = maxWidth;
      m_maxHeight = maxHeight;
    }

    bool operator==(const CVideoGeometry& rhs) const
    {
      return m_nominalWidth == rhs.m_nominalWidth && m_nominalHeight == rhs.m_nominalHeight &&
             m_maxWidth == rhs.m_maxWidth && m_maxHeight == rhs.m_maxHeight &&
             m_displayAspectRatio == rhs.m_displayAspectRatio;
    }
    bool operator!=(const CVideoGeometry& rhs) const { return !(*this == rhs); }

  private:
    unsigned int m_nominalWidth = 0;
    unsigned int m_nominalHeight = 0;
    unsigned int m_maxWidth = 0;
    unsigned int m_maxHeight = 0;
    float m_displayAspectRatio = 0.0f; // A value of 0.0f indicates square pixels
  };
}
