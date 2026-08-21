/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "AudioTiming.h"

using namespace LIBRETRO;

double CAudioTiming::GetSampleRate() const
{
  return m_sampleRate;
}

void CAudioTiming::SetSampleRate(double sampleRate)
{
  m_sampleRate = sampleRate;
}
