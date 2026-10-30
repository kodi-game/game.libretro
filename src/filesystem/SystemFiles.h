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
 * \brief Copies an emulator's system files into the system folder it is given
 *
 * The files it comes with are copied when it is installed or updated,
 * replacing any file of the same name, as RetroArch's core system files are.
 * Files put in its own system folder aren't copied, and nothing is deleted.
 */
class CSystemFiles
{
public:
  /*!
   * \param resourceDirectories The emulator's resource folders, the first
   *                            taking precedence when two have the same file
   * \param systemDirectory The system folder the emulator is given
   * \param ownDirectory The emulator's own system folder in its profile
   * \param version The emulator's version, which an update changes
   * \param recordPath Where the last copy is recorded
   *
   * \return True if the system folder has the emulator's system files
   */
  static bool Copy(const std::vector<std::string>& resourceDirectories,
                   const std::string& systemDirectory,
                   const std::string& ownDirectory,
                   const std::string& version,
                   const std::string& recordPath);
};
} // namespace LIBRETRO
