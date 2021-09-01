/*
 *  Copyright (C) 2018-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <chrono>
#include <cstdint>

namespace LIBRETRO
{
  /*!
   * \brief Utility to get the current time.
   */
  class Timer
  {
  public:
    /*!
     * \brief Returns current time in microseconds.
     */
    uint64_t microseconds();

  private:
    std::chrono::high_resolution_clock m_clock;
  };
}
