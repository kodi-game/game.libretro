/*
 *  Copyright (C) 2026 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>
#include <vector>

namespace LIBRETRO
{
/*!
 * \brief Copies an emulator's system files into the shared system folder
 *
 * With the shared folder on, an emulator is given only that folder, so the
 * files it comes with, or that were put in its own system folder, are copied
 * there. A file the shared folder already has is left alone, unless it is an
 * earlier copy and the emulator's file has changed since. Nothing is deleted.
 */
class CSharedSystemFiles
{
public:
  /*!
   * \param resourceDirectories The emulator's resource folders, the first
   *                            taking precedence when two have the same file
   * \param sharedDirectory The shared system folder
   * \param version The emulator's version, which an update changes
   * \param recordPath Where the record of the copies made is kept
   */
  static void Copy(const std::vector<std::string>& resourceDirectories,
                   const std::string& sharedDirectory,
                   const std::string& version,
                   const std::string& recordPath);
};
} // namespace LIBRETRO
