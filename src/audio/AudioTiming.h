/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

namespace LIBRETRO
{
  class CAudioTiming
  {
  public:
    double GetSampleRate() const;
    void SetSampleRate(double sampleRate);

  private:
    double m_sampleRate{0.0};
  };
} // namespace LIBRETRO
