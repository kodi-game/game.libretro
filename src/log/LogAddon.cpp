/*
 *      Copyright (C) 2014-2016 Team Kodi
 *      Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
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
