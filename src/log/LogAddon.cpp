/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LogAddon.h"

#include <kodi/General.h>

#include <assert.h>

using namespace LIBRETRO;

// --- TranslateLogLevel() -----------------------------------------------------

AddonLog TranslateLogLevel(SYS_LOG_LEVEL level)
{
  switch (level)
  {
    case SYS_LOG_ERROR: return ADDON_LOG_ERROR;
    case SYS_LOG_INFO:  return ADDON_LOG_INFO;
    case SYS_LOG_DEBUG: return ADDON_LOG_DEBUG;
    default:
      break;
  }
  return ADDON_LOG_INFO;
}

// -- CLogAddon ----------------------------------------------------------------

void CLogAddon::Log(SYS_LOG_LEVEL level, const char* logline)
{
  kodi::Log(TranslateLogLevel(level), "%s", logline);
}
