/*
 *  Copyright (C) 2020 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

namespace LIBRETRO
{
  class CCheevosEnvironment
  {
  public:
    static CCheevosEnvironment& Get(void);

    void Initialize();

    void Deinitialize();

  private:
    CCheevosEnvironment() = default;
  };
} // namespace LIBRETRO
