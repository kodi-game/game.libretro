/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "VideoTiming.h"

#include "libretro-common/libretro.h"

#include <cmath>

using namespace LIBRETRO;

bool CVideoTiming::IsFastForwarding(double playbackSpeed)
{
  return playbackSpeed > 1.0;
}

void CVideoTiming::GetThrottleState(double playbackSpeed,
                                    retro_throttle_state& throttleState) const
{
  // Kodi's speed carries the player's meaning, so the mode falls out of it.
  // Rewinding is reported for a negative speed even though the core is still
  // run forwards, because Kodi rewinds by replaying saved states around a core
  // that only knows how to go forwards.
  if (playbackSpeed < 0.0)
    throttleState.mode = RETRO_THROTTLE_REWINDING;
  else if (playbackSpeed == 0.0)
    throttleState.mode = RETRO_THROTTLE_FRAME_STEPPING;
  else if (playbackSpeed > 1.0)
    throttleState.mode = RETRO_THROTTLE_FAST_FORWARD;
  else if (playbackSpeed < 1.0)
    throttleState.mode = RETRO_THROTTLE_SLOW_MOTION;
  else
    throttleState.mode = RETRO_THROTTLE_VSYNC;

  // How often the core is meant to run, which is its own rate scaled by how
  // fast the user is going. Zero is the documented answer for "no known fixed
  // rate", and is what a core gets before it has declared one.
  throttleState.rate = static_cast<float>(m_frameRate * std::abs(playbackSpeed));
}
