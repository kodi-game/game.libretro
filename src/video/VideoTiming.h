/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

struct retro_throttle_state;

namespace LIBRETRO
{
  class CVideoTiming
  {
  public:
    // Timing interface
    double GetFrameRate() const { return m_frameRate; }
    void SetFrameRate(double frameRate) { m_frameRate = frameRate; }

    // Libretro interface
    static bool IsFastForwarding(double playbackSpeed);
    void GetThrottleState(double playbackSpeed, retro_throttle_state& throttleState) const;

  private:
    double m_frameRate{0.0};
  };
} // namespace LIBRETRO
