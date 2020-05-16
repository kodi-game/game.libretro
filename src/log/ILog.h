/*
 *  Copyright (C) 2014-2020 Team Kodi (https://kodi.tv)
 *  Portions Copyright (C) 2013-2014 Lars Op den Kamp
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

namespace LIBRETRO
{
  enum SYS_LOG_LEVEL
  {
    SYS_LOG_NONE = 0,
    SYS_LOG_ERROR,
    SYS_LOG_INFO,
    SYS_LOG_DEBUG
  };

  enum SYS_LOG_TYPE
  {
    SYS_LOG_TYPE_NULL,     // Discard log
    SYS_LOG_TYPE_CONSOLE,  // Log to stdout
    SYS_LOG_TYPE_ADDON     // Log to frontend
  };

  /*!
   * \brief Logging interface
   */
  class ILog
  {
  public:
    virtual ~ILog(void) { }

    /*!
     * \brief Send a message to the log
     */
    virtual void Log(SYS_LOG_LEVEL level, const char* logline) = 0;

    /*!
     * \brief Get the type of logging used in the implementation
     */
    virtual SYS_LOG_TYPE Type(void) const = 0;
  };
}
