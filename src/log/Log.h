/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "ILog.h"

#include <mutex>

#include <string>

// --- Shorthand logging -------------------------------------------------------

#ifndef esyslog
  #define esyslog(...) LIBRETRO::CLog::Get().Log(SYS_LOG_ERROR, __VA_ARGS__)
#endif

#ifndef isyslog
  #define isyslog(...) LIBRETRO::CLog::Get().Log(SYS_LOG_INFO, __VA_ARGS__)
#endif

#ifndef dsyslog
  #define dsyslog(...) LIBRETRO::CLog::Get().Log(SYS_LOG_DEBUG, __VA_ARGS__)
#endif

#define LOG_ERROR_STR(str)  esyslog("ERROR (%s, %d): %s: %m", __FILE__, __LINE__, str)

// --- CLog --------------------------------------------------------------------

namespace LIBRETRO
{
  class CLog
  {
  private:
    CLog(ILog* pipe);

  public:
    static CLog& Get(void);
    ~CLog(void);

    bool SetType(SYS_LOG_TYPE type);
    void SetPipe(ILog* pipe);
    void SetLevel(SYS_LOG_LEVEL level);
    void SetLogPrefix(const std::string& strLogPrefix);

    void Log(SYS_LOG_LEVEL level, const char* format, ...);

    static const char* TypeToString(SYS_LOG_TYPE type);

  private:
    static const char* GetLogPrefix(SYS_LOG_LEVEL level);

    ILog*            m_pipe;
    SYS_LOG_LEVEL    m_level;
    std::string      m_strLogPrefix;
    std::mutex       m_mutex;
  };
}
