/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Timer.h"

using namespace LIBRETRO;

uint64_t Timer::microseconds()
{
  const auto currenttime = m_clock.now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(currenttime).count();
}
