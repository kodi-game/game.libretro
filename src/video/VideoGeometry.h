/*
 *  Copyright (C) 2018-2020 Team Kodi (https://kodi.tv)
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
    float AspectRatio() const { return m_aspectRatio; }

  private:
    unsigned int m_nominalWidth = 0;
    unsigned int m_nominalHeight = 0;
    unsigned int m_maxWidth = 0;
    unsigned int m_maxHeight = 0;
    float m_aspectRatio = 0.0f;
  };
}
