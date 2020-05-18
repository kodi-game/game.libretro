/*
 *  Copyright (C) 2018-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "VideoGeometry.h"
#include "libretro/libretro.h"

using namespace LIBRETRO;

CVideoGeometry::CVideoGeometry(const retro_game_geometry &geometry)
{
  UpdateVideoGeometry(geometry);
}

void CVideoGeometry::UpdateVideoGeometry(const retro_game_geometry &geometry)
{
  m_nominalWidth = geometry.base_width;
  m_nominalHeight = geometry.base_height;
  m_maxWidth = geometry.max_width;
  m_maxHeight = geometry.max_height;
  m_aspectRatio = geometry.aspect_ratio;
}
