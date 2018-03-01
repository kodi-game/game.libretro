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
    float AspectRatio() const { return m_aspectRatio; }

  private:
    unsigned int m_nominalWidth = 0;
    unsigned int m_nominalHeight = 0;
    unsigned int m_maxWidth = 0;
    unsigned int m_maxHeight = 0;
    float m_aspectRatio = 0.0f;
  };
}
