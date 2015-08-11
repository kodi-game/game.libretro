/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "LogAddon.h"

#include "kodi/libXBMC_addon.h"

#include <assert.h>

using namespace LIBRETRO;

// --- TranslateLogLevel() -----------------------------------------------------

ADDON::addon_log_t TranslateLogLevel(SYS_LOG_LEVEL level)
{
  switch (level)
  {
    case SYS_LOG_ERROR: return ADDON::LOG_ERROR;
    case SYS_LOG_INFO:  return ADDON::LOG_INFO;
    case SYS_LOG_DEBUG: return ADDON::LOG_DEBUG;
    default:
      break;
  }
  return ADDON::LOG_INFO;
}

// -- CLogAddon ----------------------------------------------------------------

CLogAddon::CLogAddon(ADDON::CHelper_libXBMC_addon* frontend) :
  m_frontend(frontend)
{
  assert(m_frontend);
}

void CLogAddon::Log(SYS_LOG_LEVEL level, const char* logline)
{
  if (m_frontend)
    m_frontend->Log(TranslateLogLevel(level), logline);
}
