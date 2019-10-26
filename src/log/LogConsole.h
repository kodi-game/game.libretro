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
#pragma once

#include "ILog.h"

#include <mutex>

namespace LIBRETRO
{
  class CLogConsole : public ILog
  {
  public:
    virtual ~CLogConsole(void) { }

    // implementation of ILog
    virtual void Log(SYS_LOG_LEVEL level, const char* logline);
    virtual SYS_LOG_TYPE Type(void) const { return SYS_LOG_TYPE_CONSOLE; }

  private:
    std::mutex m_mutex;
  };
}
