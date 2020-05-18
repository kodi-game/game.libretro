/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "LogConsole.h"

#include <iostream>

using namespace LIBRETRO;

void CLogConsole::Log(SYS_LOG_LEVEL level, const char* logline)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  // TODO: Prepend current date
  std::cout << logline << std::endl;
}
