/*
 *  Copyright (C) 2014-2021 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Log.h"
#include "LogAddon.h"
#include "LogConsole.h"

#include <stdarg.h>
#include <stdio.h>

using namespace LIBRETRO;

#define SYS_LOG_BUFFER_SIZE  256 // bytes

CLog::CLog(ILog* pipe) :
  m_pipe(pipe),
  m_level(SYS_LOG_DEBUG)
{
}

CLog& CLog::Get(void)
{
  static CLog _instance(new CLogConsole);
  return _instance;
}

CLog::~CLog(void)
{
  SetPipe(nullptr);
}

bool CLog::SetType(SYS_LOG_TYPE type)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  if (m_pipe && m_pipe->Type() == type)
    return true; // Already set

  switch (type)
  {
  case SYS_LOG_TYPE_CONSOLE:
    SetPipe(new CLogConsole);
    break;
  case SYS_LOG_TYPE_NULL:
    SetPipe(nullptr);
    break;
  case SYS_LOG_TYPE_ADDON: // Must be set through SetPipe() because CLogAddon has no default constructor
  default:
    Log(SYS_LOG_ERROR, "Failed to set log type to %s", TypeToString(type));
    return false;
  }

  return true;
}

void CLog::SetPipe(ILog* pipe)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  delete m_pipe;
  m_pipe = pipe;
}

void CLog::SetLevel(SYS_LOG_LEVEL level)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  m_level = level;
}

void CLog::SetLogPrefix(const std::string& strLogPrefix)
{
  m_strLogPrefix = strLogPrefix;
}

void CLog::Log(SYS_LOG_LEVEL level, const char* format, ...)
{
  std::string strLogPrefix;

  if (m_pipe && m_pipe->Type() == SYS_LOG_TYPE_CONSOLE)
    strLogPrefix = GetLogPrefix(level) + m_strLogPrefix;
  else
    strLogPrefix = m_strLogPrefix;

  char fmt[SYS_LOG_BUFFER_SIZE];
  char buf[SYS_LOG_BUFFER_SIZE];
  va_list ap;

  va_start(ap, format);
  snprintf(fmt, sizeof(fmt), "%s%s", strLogPrefix.c_str(), format);
  vsnprintf(buf, SYS_LOG_BUFFER_SIZE - 1, fmt, ap);
  va_end(ap);

  std::unique_lock<std::mutex> lock(m_mutex);

  if (level > m_level)
    return;

  if (m_pipe)
    m_pipe->Log(level, buf);
}

const char* CLog::TypeToString(SYS_LOG_TYPE type)
{
  switch (type)
  {
  case SYS_LOG_TYPE_NULL:
    return "null";
  case SYS_LOG_TYPE_CONSOLE:
    return "console";
  case SYS_LOG_TYPE_ADDON:
    return "addon";
  default:
    return "unknown";
  }
}

const char* CLog::GetLogPrefix(SYS_LOG_LEVEL level)
{
  switch (level)
  {
  case SYS_LOG_ERROR:   return "[ERROR] ";
  case SYS_LOG_INFO:    return "[INFO]  ";
  case SYS_LOG_DEBUG:   return "[DEBUG] ";
  case SYS_LOG_NONE:
  default:              return "[?????] ";
  }
}
