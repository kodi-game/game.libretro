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
