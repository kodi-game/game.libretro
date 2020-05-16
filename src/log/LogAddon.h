/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "ILog.h"

namespace LIBRETRO
{
  class CLogAddon : public ILog
  {
  public:
    CLogAddon() = default;
    virtual ~CLogAddon(void) { }

    // implementation of ILog
    virtual void Log(SYS_LOG_LEVEL level, const char* logline);
    virtual SYS_LOG_TYPE Type(void) const { return SYS_LOG_TYPE_ADDON; }
  };
}
