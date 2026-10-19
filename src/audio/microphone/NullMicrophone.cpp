/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "IMicrophone.h"

using namespace LIBRETRO;

std::unique_ptr<IMicrophone> LIBRETRO::CreateMicrophone()
{
  return {};
}

bool LIBRETRO::CanCapture()
{
  return false;
}
