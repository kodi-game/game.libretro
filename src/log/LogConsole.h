/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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
